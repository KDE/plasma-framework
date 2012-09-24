/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Ménard Alexis <darktears31@gmail.com>
 *   Copyright 2009 Chani Armitage <chani@kde.org>
 *   Copyright 2012 Marco Martin <notmart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "private/containment_p.h"

#include <QApplication>
#include <QClipboard>
#include <QGraphicsSceneDragDropEvent>
#include <QMimeData>
#include <QMimeDatabase>
#include <QGraphicsView>
#include <qtemporaryfile.h>

#include <kaction.h>
#include <kactioncollection.h>
#include <kcoreauthorized.h>
#include <kurlmimedata.h>
#include <kwindowsystem.h>

#include "config-plasma.h"

#if !PLASMA_NO_KIO
#include "kio/jobclasses.h" // for KIO::JobFlags
#include "kio/job.h"
#include "kio/scheduler.h"
#endif

#include "abstracttoolbox.h"
#include "containmentactions.h"
#include "containmentactionspluginsconfig.h"
#include "corona.h"
#include "pluginloader.h"
#include "svg.h"
#include "wallpaper.h"

#include "remote/accessappletjob.h"
#include "remote/accessmanager.h"

#include "private/applet_p.h"
#include "private/containmentactionspluginsconfig_p.h"
#include "private/wallpaper_p.h"


namespace Plasma
{

bool ContainmentPrivate::s_positioningPanels = false;
QHash<QString, ContainmentActions*> ContainmentPrivate::globalActionPlugins;

const char ContainmentPrivate::defaultWallpaper[] = "image";
const char ContainmentPrivate::defaultWallpaperMode[] = "SingleImage";

void ContainmentPrivate::addDefaultActions(KActionCollection *actions, Containment *c)
{
    actions->setConfigGroup("Shortcuts-Containment");

    //adjust applet actions
    KAction *appAction = qobject_cast<KAction*>(actions->action("remove"));
    appAction->setShortcut(KShortcut("alt+d, alt+r"));
    if (c && c->d->isPanelContainment()) {
        appAction->setText(i18n("Remove this Panel"));
    } else {
        appAction->setText(i18n("Remove this Activity"));
    }

    appAction = qobject_cast<KAction*>(actions->action("configure"));
    if (appAction) {
        appAction->setShortcut(KShortcut("alt+d, alt+s"));
        appAction->setText(i18n("Activity Settings"));
    }

    //add our own actions
    KAction *appletBrowserAction = actions->addAction("add widgets");
    appletBrowserAction->setAutoRepeat(false);
    appletBrowserAction->setText(i18n("Add Widgets..."));
    appletBrowserAction->setIcon(KDE::icon("list-add"));
    appletBrowserAction->setShortcut(KShortcut("alt+d, a"));
    appletBrowserAction->setData(AbstractToolBox::AddTool);

    KAction *action = actions->addAction("next applet");
    action->setText(i18n("Next Widget"));
    //no icon
    action->setShortcut(KShortcut("alt+d, n"));
    action->setData(AbstractToolBox::ControlTool);

    action = actions->addAction("previous applet");
    action->setText(i18n("Previous Widget"));
    //no icon
    action->setShortcut(KShortcut("alt+d, p"));
    action->setData(AbstractToolBox::ControlTool);
}

void ContainmentPrivate::initApplets()
{
    foreach (Applet *applet, applets) {
        applet->restore(*applet->d->mainConfigGroup());
        applet->init();
#ifndef NDEBUG
        kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Applet" << applet->name();
#endif
    }

    q->flushPendingConstraintsEvents();

    foreach (Applet *applet, applets) {
        applet->flushPendingConstraintsEvents();
    }

#ifndef NDEBUG
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "Containment's applets initialized" << q->name();
#endif
}

void ContainmentPrivate::checkContainmentFurniture()
{
    if (q->isContainment() &&
        (type == Containment::DesktopContainment || type == Containment::PanelContainment)) {
        createToolBox();
    }
}

void ContainmentPrivate::addContainmentActions(KMenu &desktopMenu, QEvent *event)
{
    if (q->corona()->immutability() != Mutable &&
        !KAuthorized::authorizeKAction("plasma/containment_actions")) {
        //kDebug() << "immutability";
        return;
    }

    const QString trigger = ContainmentActions::eventToString(event);
    prepareContainmentActions(trigger, QPoint(), &desktopMenu);
}

void ContainmentPrivate::addAppletActions(KMenu &desktopMenu, Applet *applet, QEvent *event)
{
    foreach (QAction *action, applet->contextualActions()) {
        if (action) {
            desktopMenu.addAction(action);
        }
    }

    if (!applet->d->failed) {
        QAction *configureApplet = applet->d->actions->action("configure");
        if (configureApplet && configureApplet->isEnabled()) {
            desktopMenu.addAction(configureApplet);
        }

        QAction *runAssociatedApplication = applet->d->actions->action("run associated application");
        if (runAssociatedApplication && runAssociatedApplication->isEnabled()) {
            desktopMenu.addAction(runAssociatedApplication);
        }
    }

    KMenu *containmentMenu = new KMenu(i18nc("%1 is the name of the containment", "%1 Options", q->name()), &desktopMenu);
    addContainmentActions(*containmentMenu, event);
    if (!containmentMenu->isEmpty()) {
        int enabled = 0;
        //count number of real actions
        QListIterator<QAction *> actionsIt(containmentMenu->actions());
        while (enabled < 3 && actionsIt.hasNext()) {
            QAction *action = actionsIt.next();
            if (action->isVisible() && !action->isSeparator()) {
                ++enabled;
            }
        }

        if (enabled) {
            //if there is only one, don't create a submenu
            if (enabled < 2) {
                foreach (QAction *action, containmentMenu->actions()) {
                    if (action->isVisible() && !action->isSeparator()) {
                        desktopMenu.addAction(action);
                    }
                }
            } else {
                desktopMenu.addMenu(containmentMenu);
            }
        }
    }

    if (q->immutability() == Mutable) {
        QAction *closeApplet = applet->d->actions->action("remove");
        //kDebug() << "checking for removal" << closeApplet;
        if (closeApplet) {
            if (!desktopMenu.isEmpty()) {
                desktopMenu.addSeparator();
            }

            //kDebug() << "adding close action" << closeApplet->isEnabled() << closeApplet->isVisible();
            desktopMenu.addAction(closeApplet);
        }
    }
}

Applet* ContainmentPrivate::appletAt(const QPointF &point)
{
  return 0;
  /*TODO: port away qgv
    Applet *applet = 0;

    QGraphicsItem *item = q->scene()->itemAt(point);
    if (item == q) {
        item = 0;
    }

    while (item) {
        if (item->isWidget()) {
            applet = qobject_cast<Applet*>(static_cast<QGraphicsWidget*>(item));
            if (applet) {
                if (applet->isContainment()) {
                    applet = 0;
                }
                break;
            }
        }
        AppletHandle *handle = dynamic_cast<AppletHandle*>(item);
        if (handle) {
            //pretend it was on the applet
            applet = handle->applet();
            break;
        }
        item = item->parentItem();
    }
    return applet;
    */
}

void ContainmentPrivate::setScreen(int newScreen, int newDesktop, bool preventInvalidDesktops)
{
    // What we want to do in here is:
    //   * claim the screen as our own
    //   * signal whatever may be watching this containment about the switch
    //   * if we are a full screen containment, then:
    //      * resize to match the screen if we're that kind of containment
    //      * kick other full-screen containments off this screen
    //          * if we had a screen, then give our screen to the containment
    //            we kick out
    //
    // a screen of -1 means no associated screen.
    Corona *corona = q->corona();
    Q_ASSERT(corona);

    //if it's an offscreen widget, don't allow to claim a screen, after all it's *off*screen
    //TODO: port away qgv
    /* should decide in a different way if this is a dashboard containment
    if (corona->offscreenWidgets().contains(q)) {
        return;
    }*/

    int numScreens = corona->numScreens();
    if (newScreen < -1) {
        newScreen = -1;
    }

    // -1 == All desktops
    if (newDesktop < -1 || (preventInvalidDesktops && newDesktop > KWindowSystem::numberOfDesktops() - 1)) {
        newDesktop = -1;
    }

    //kDebug() << activity() << "setting screen to " << newScreen << newDesktop << "and type is" << type;

    Containment *swapScreensWith(0);
    const bool isDesktopContainment = type == Containment::DesktopContainment ||
                                      type == Containment::CustomContainment;
    if (isDesktopContainment) {
        // we want to listen to changes in work area if our screen changes
        if (toolBox) {
            if (screen < 0 && newScreen > -1) {
                QObject::connect(KWindowSystem::self(), SIGNAL(workAreaChanged()), toolBox.data(), SLOT(reposition()), Qt::UniqueConnection);
            } else if (newScreen < 0) {
                QObject::disconnect(KWindowSystem::self(), SIGNAL(workAreaChanged()), toolBox.data(), SLOT(reposition()));
            }
        }

        if (newScreen > -1) {
            // sanity check to make sure someone else doesn't have this screen already!
            Containment *currently = corona->containmentForScreen(newScreen, newDesktop);
            if (currently && currently != q) {
#ifndef NDEBUG
                kDebug() << "currently is on screen" << currently->screen()
                         << "desktop" << currently->desktop()
                         << "and is" << currently->activity()
                         << (QObject*)currently << "i'm" << (QObject*)q;
#endif
                currently->setScreen(-1, currently->desktop());
                swapScreensWith = currently;
            }
        }
    }

    if (newScreen < numScreens && newScreen > -1 && isDesktopContainment) {
        q->resize(corona->screenGeometry(newScreen).size());
    }

    int oldDesktop = desktop;
    desktop = newDesktop;

    int oldScreen = screen;
    screen = newScreen;

    q->updateConstraints(Plasma::ScreenConstraint);

    if (oldScreen != newScreen || oldDesktop != newDesktop) {
        /*
#ifndef NDEBUG
        kDebug() << "going to signal change for" << q
#endif
                 << ", old screen & desktop:" << oldScreen << oldDesktop
                 << ", new:" << screen << desktop;
                 */
        KConfigGroup c = q->config();
        c.writeEntry("screen", screen);
        c.writeEntry("desktop", desktop);
        if (newScreen != -1) {
            lastScreen = newScreen;
            lastDesktop = newDesktop;
            c.writeEntry("lastScreen", lastScreen);
            c.writeEntry("lastDesktop", lastDesktop);
        }
        emit q->configNeedsSaving();
        emit q->screenChanged(oldScreen, newScreen, q);
    }

    if (swapScreensWith) {
        //kDebug() << "setScreen due to swap, part 2";
        swapScreensWith->setScreen(oldScreen, oldDesktop);
    }

    checkRemoveAction();

    if (newScreen >= 0) {
        emit q->activate();
    }
}

void ContainmentPrivate::showDropZoneDelayed()
{
    dropZoneStarted = true;
    q->showDropZone(dropPoints.value(0).toPoint());
    dropPoints.remove(0);
}

void ContainmentPrivate::dropData(QPointF scenePos, QPoint screenPos, QGraphicsSceneDragDropEvent *dropEvent)
{
    if (q->immutability() != Mutable) {
        return;
    }

    const QMimeData *mimeData = 0;

    if (dropEvent) {
        mimeData = dropEvent->mimeData();
    } else {
        QClipboard *clipboard = QApplication::clipboard();
        mimeData = clipboard->mimeData(QClipboard::Selection);
        //TODO if that's not supported (ie non-linux) should we try clipboard instead of selection?
    }

    if (!mimeData) {
        //Selection is either empty or not supported on this OS
#ifndef NDEBUG
        kDebug() << "no mime data";
#endif
        return;
    }

    //kDebug() << event->mimeData()->text();

    QString appletMimetype(q->corona() ? q->corona()->appletMimeType() : QString());

    if (!appletMimetype.isEmpty() && mimeData->hasFormat(appletMimetype)) {
        QString data = mimeData->data(appletMimetype);
        const QStringList appletNames = data.split('\n', QString::SkipEmptyParts);
        foreach (const QString &appletName, appletNames) {
            //kDebug() << "doing" << appletName;
            QRectF geom(scenePos, QSize(0, 0));
            q->addApplet(appletName, QVariantList(), geom);
        }
        if (dropEvent) {
            dropEvent->acceptProposedAction();
        }
    } else if (mimeData->hasUrls()) {
        //TODO: collect the mimeTypes of available script engines and offer
        //      to create widgets out of the matching URLs, if any
        const QList<QUrl> urls = KUrlMimeData::urlsFromMimeData(mimeData);
        foreach (const QUrl &url, urls) {
            if (AccessManager::supportedProtocols().contains(url.scheme())) {
                AccessAppletJob *job = AccessManager::self()->accessRemoteApplet(url);
                if (dropEvent) {
                    dropPoints[job] = dropEvent->pos();
                } else {
                    dropPoints[job] = scenePos;
                }
                QObject::connect(AccessManager::self(), SIGNAL(finished(Plasma::AccessAppletJob*)),
                                 q, SLOT(remoteAppletReady(Plasma::AccessAppletJob*)));
            }
#if !PLASMA_NO_KIO
            else {
                QMimeDatabase db;
                QMimeType mime = db.mimeTypeForUrl(url);
                QString mimeName = mime.name();
                QRectF geom(scenePos, QSize());
                QVariantList args;
                args << url.toString();
#ifndef NDEBUG
                kDebug() << "can decode" << mimeName << args;
#endif

                // It may be a directory or a file, let's stat
                KIO::JobFlags flags = KIO::HideProgressInfo;
                KIO::MimetypeJob *job = KIO::mimetype(url, flags);
                if (dropEvent) {
                    dropPoints[job] = dropEvent->pos();
                } else {
                    dropPoints[job] = scenePos;
                }

                QObject::connect(job, SIGNAL(result(KJob*)), q, SLOT(dropJobResult(KJob*)));
                QObject::connect(job, SIGNAL(mimetype(KIO::Job*,QString)),
                                 q, SLOT(mimeTypeRetrieved(KIO::Job*,QString)));

                KMenu *choices = new KMenu("Content dropped");
                choices->addAction(KDE::icon("process-working"), i18n("Fetching file type..."));
                if (dropEvent) {
                    choices->popup(dropEvent->screenPos());
                } else {
                    choices->popup(screenPos);
                }

                dropMenus[job] = choices;
            }
#endif
        }

        if (dropEvent) {
            dropEvent->acceptProposedAction();
        }
    } else {
        QStringList formats = mimeData->formats();
        QHash<QString, KPluginInfo> seenPlugins;
        QHash<QString, QString> pluginFormats;

        foreach (const QString &format, formats) {
            KPluginInfo::List plugins = Applet::listAppletInfoForMimeType(format);

            foreach (const KPluginInfo &plugin, plugins) {
                if (seenPlugins.contains(plugin.pluginName())) {
                    continue;
                }

                seenPlugins.insert(plugin.pluginName(), plugin);
                pluginFormats.insert(plugin.pluginName(), format);
            }
        }
        //kDebug() << "Mimetype ..." << formats << seenPlugins.keys() << pluginFormats.values();

        QString selectedPlugin;

        if (seenPlugins.isEmpty()) {
            // do nothing
        } else if (seenPlugins.count() == 1) {
            selectedPlugin = seenPlugins.constBegin().key();
        } else {
            KMenu choices;
            QHash<QAction *, QString> actionsToPlugins;
            foreach (const KPluginInfo &info, seenPlugins) {
                QAction *action;
                if (!info.icon().isEmpty()) {
                    action = choices.addAction(KDE::icon(info.icon()), info.name());
                } else {
                    action = choices.addAction(info.name());
                }

                actionsToPlugins.insert(action, info.pluginName());
            }

            QAction *choice = choices.exec(screenPos);
            if (choice) {
                selectedPlugin = actionsToPlugins[choice];
            }
        }

        if (!selectedPlugin.isEmpty()) {
            if (!dropEvent) {
                // since we may have entered an event loop up above with the menu,
                // the clipboard item may no longer be valid, as QClipboard resets
                // the object behind the back of the application with a zero timer
                // so we fetch it again here
                QClipboard *clipboard = QApplication::clipboard();
                mimeData = clipboard->mimeData(QClipboard::Selection);
            }

            QTemporaryFile tempFile;
            if (mimeData && tempFile.open()) {
                //TODO: what should we do with files after the applet is done with them??
                tempFile.setAutoRemove(false);

                {
                    QDataStream stream(&tempFile);
                    QByteArray data = mimeData->data(pluginFormats[selectedPlugin]);
                    stream.writeRawData(data, data.size());
                }

                QRectF geom(scenePos, QSize());
                QVariantList args;
                args << tempFile.fileName();
#ifndef NDEBUG
                kDebug() << args;
#endif
                tempFile.close();

                q->addApplet(selectedPlugin, args, geom);
            }
        }
    }
}

void ContainmentPrivate::clearDataForMimeJob(KIO::Job *job)
{
#if !PLASMA_NO_KIO
    QObject::disconnect(job, 0, q, 0);
    dropPoints.remove(job);
    KMenu *choices = dropMenus.take(job);
    delete choices;
    job->kill();
#endif // PLASMA_NO_KIO
}

void ContainmentPrivate::remoteAppletReady(Plasma::AccessAppletJob *job)
{
    QPointF pos = dropPoints.take(job);
    if (job->error()) {
        //TODO: nice user visible error handling (knotification probably?)
#ifndef NDEBUG
        kDebug() << "remote applet access failed: " << job->errorText();
#endif
        return;
    }

    if (!job->applet()) {
#ifndef NDEBUG
        kDebug() << "how did we end up here? if applet is null, the job->error should be nonzero";
#endif
        return;
    }

    q->addApplet(job->applet(), pos);
}

void ContainmentPrivate::dropJobResult(KJob *job)
{
#if !PLASMA_NO_KIO
    KIO::TransferJob* tjob = dynamic_cast<KIO::TransferJob*>(job);
    if (!tjob) {
#ifndef NDEBUG
        kDebug() << "job is not a KIO::TransferJob, won't handle the drop...";
#endif
        clearDataForMimeJob(tjob);
        return;
    }
    if (job->error()) {
#ifndef NDEBUG
        kDebug() << "ERROR" << tjob->error() << ' ' << tjob->errorString();
#endif
    }
    // We call mimeTypeRetrieved since there might be other mechanisms
    // for finding suitable applets. Cleanup happens there as well.
    mimeTypeRetrieved(qobject_cast<KIO::Job *>(job), QString());
#endif // PLASMA_NO_KIO
}

void ContainmentPrivate::mimeTypeRetrieved(KIO::Job *job, const QString &mimeType)
{
#if !PLASMA_NO_KIO
#ifndef NDEBUG
    kDebug() << "Mimetype Job returns." << mimeType;
#endif
    KIO::TransferJob* tjob = dynamic_cast<KIO::TransferJob*>(job);
    if (!tjob) {
#ifndef NDEBUG
        kDebug() << "job should be a TransferJob, but isn't";
#endif
        clearDataForMimeJob(job);
        return;
    }
    KPluginInfo::List appletList = Applet::listAppletInfoForUrl(tjob->url());
    if (mimeType.isEmpty() && !appletList.count()) {
        clearDataForMimeJob(job);
#ifndef NDEBUG
        kDebug() << "No applets found matching the url (" << tjob->url() << ") or the mimeType (" << mimeType << ")";
#endif
        return;
    } else {

        QPointF posi; // will be overwritten with the event's position
        if (dropPoints.keys().contains(tjob)) {
            posi = dropPoints[tjob];
#ifndef NDEBUG
            kDebug() << "Received a suitable dropEvent at" << posi;
#endif
        } else {
#ifndef NDEBUG
            kDebug() << "Bailing out. Cannot find associated dropEvent related to the TransferJob";
#endif
            clearDataForMimeJob(job);
            return;
        }

        KMenu *choices = dropMenus.value(tjob);
        if (!choices) {
#ifndef NDEBUG
            kDebug() << "Bailing out. No QMenu found for this job.";
#endif
            clearDataForMimeJob(job);
            return;
        }

        QVariantList args;
        args << tjob->url().toString() << mimeType;

#ifndef NDEBUG
        kDebug() << "Creating menu for:" << mimeType  << posi << args;
#endif

        appletList << Applet::listAppletInfoForMimeType(mimeType);
        KPluginInfo::List wallpaperList;
        if (drawWallpaper) {
            if (wallpaper && wallpaper->supportsMimetype(mimeType)) {
                wallpaperList << wallpaper->d->wallpaperDescription;
            } else {
                wallpaperList = Wallpaper::listWallpaperInfoForMimeType(mimeType);
            }
        }

        if (!appletList.isEmpty() || !wallpaperList.isEmpty()) {
            choices->clear();
            QHash<QAction *, QString> actionsToApplets;
            choices->addTitle(i18n("Widgets"));
            foreach (const KPluginInfo &info, appletList) {
#ifndef NDEBUG
                kDebug() << info.name();
#endif
                QAction *action;
                if (!info.icon().isEmpty()) {
                    action = choices->addAction(KDE::icon(info.icon()), info.name());
                } else {
                    action = choices->addAction(info.name());
                }

                actionsToApplets.insert(action, info.pluginName());
#ifndef NDEBUG
                kDebug() << info.pluginName();
#endif
            }
            actionsToApplets.insert(choices->addAction(i18n("Icon")), "icon");

            QHash<QAction *, QString> actionsToWallpapers;
            if (!wallpaperList.isEmpty())  {
                choices->addTitle(i18n("Wallpaper"));

                QMap<QString, KPluginInfo> sorted;
                foreach (const KPluginInfo &info, appletList) {
                    sorted.insert(info.name(), info);
                }

                foreach (const KPluginInfo &info, wallpaperList) {
                    QAction *action;
                    if (!info.icon().isEmpty()) {
                        action = choices->addAction(KDE::icon(info.icon()), info.name());
                    } else {
                        action = choices->addAction(info.name());
                    }

                    actionsToWallpapers.insert(action, info.pluginName());
                }
            }

            QAction *choice = choices->exec();
            if (choice) {
                // Put the job on hold so it can be recycled to fetch the actual content,
                // which is to be expected when something's dropped onto the desktop and
                // an applet is to be created with this URL
                if (!mimeType.isEmpty() && !tjob->error()) {
                    tjob->putOnHold();
                    KIO::Scheduler::publishSlaveOnHold();
                }
                QString plugin = actionsToApplets.value(choice);
                if (plugin.isEmpty()) {
                    //set wallpapery stuff
                    plugin = actionsToWallpapers.value(choice);
                    if (!wallpaper || plugin != wallpaper->pluginName()) {
                        //kDebug() << "Wallpaper dropped:" << tjob->url();
                        q->setWallpaper(plugin);
                    }

                    if (wallpaper) {
                        //kDebug() << "Wallpaper dropped:" << tjob->url();
                        wallpaper->addUrls(QList<QUrl>() << tjob->url());
                    }
                } else {
                    addApplet(actionsToApplets[choice], args, QRectF(posi, QSize()));
                }

                clearDataForMimeJob(job);
                return;
            }
        } else {
            // we can at least create an icon as a link to the URL
            addApplet("icon", args, QRectF(posi, QSize()));
        }
    }

    clearDataForMimeJob(job);
#endif // PLASMA_NO_KIO
}

KActionCollection* ContainmentPrivate::actions()
{
    return static_cast<Applet*>(q)->d->actions;
}

void ContainmentPrivate::focusApplet(Plasma::Applet *applet)
{
    if (focusedApplet == applet) {
        return;
    }

    QList<QWidget *> widgets = actions()->associatedWidgets();
    if (focusedApplet) {
        foreach (QWidget *w, widgets) {
            focusedApplet->removeAssociatedWidget(w);
        }
    }

    if (applet && applets.contains(applet)) {
        //kDebug() << "switching to" << applet->name();
        focusedApplet = applet;
        foreach (QWidget *w, widgets) {
            focusedApplet->addAssociatedWidget(w);
        }

        if (!focusedApplet->hasFocus()) {
            focusedApplet->setFocus(Qt::ShortcutFocusReason);
        }
    } else {
        focusedApplet = 0;
    }
}

void ContainmentPrivate::configChanged()
{
    if (drawWallpaper) {
        KConfigGroup group = q->config();
        q->setWallpaper(group.readEntry("wallpaperplugin", defaultWallpaper),
                        group.readEntry("wallpaperpluginmode", defaultWallpaperMode));
    }
}

void ContainmentPrivate::requestConfiguration()
{
    emit q->configureRequested(q);
}

void ContainmentPrivate::checkStatus(Plasma::ItemStatus appletStatus)
{
    //kDebug() << "================== "<< appletStatus << q->status();
    if (appletStatus == q->status()) {
        emit q->newStatus(appletStatus);
        return;
    }

    if (appletStatus < q->status()) {
        // check to see if any other applet has a higher status, and stick with that
        // if we do
        foreach (Applet *applet, applets) {
            if (applet->status() > appletStatus) {
                appletStatus = applet->status();
            }
        }
    }

    q->setStatus(appletStatus);
}

void ContainmentPrivate::createToolBox()
{
    if (!toolBox && KAuthorized::authorizeKAction("plasma/containment_context_menu")) {
        toolBox = Plasma::AbstractToolBox::load(q->corona()->preferredToolBoxPlugin(type), QVariantList(), q);

        if (toolBox) {
            QObject::connect(toolBox.data(), SIGNAL(toggled()), q, SIGNAL(toolBoxToggled()));
            QObject::connect(toolBox.data(), SIGNAL(toggled()), q, SLOT(updateToolBoxVisibility()));

            positionToolBox();
        }
    }
}

void ContainmentPrivate::positionToolBox()
{
    if (toolBox) {
        toolBox.data()->reposition();
    }
}

void ContainmentPrivate::updateToolBoxVisibility()
{
    emit q->toolBoxVisibilityChanged(toolBox.data()->isShowing());
}

void ContainmentPrivate::triggerShowAddWidgets()
{
    emit q->showAddWidgetsInterface(QPointF());
}

void ContainmentPrivate::containmentConstraintsEvent(Plasma::Constraints constraints)
{
    if (!q->isContainment()) {
        return;
    }

    //kDebug() << "got containmentConstraintsEvent" << constraints << (QObject*)toolBox;
    if (constraints & Plasma::ImmutableConstraint) {
        //update actions
        checkRemoveAction();
        const bool unlocked = q->immutability() == Mutable;
        q->setAcceptDrops(unlocked);
        q->enableAction("add widgets", unlocked);

        // tell the applets too
        foreach (Applet *a, applets) {
            a->setImmutability(q->immutability());
            a->updateConstraints(ImmutableConstraint);
        }
    }

    // pass on the constraints that are relevant here
    Constraints appletConstraints = NoConstraint;
    if (constraints & FormFactorConstraint) {
        appletConstraints |= FormFactorConstraint;
    }

    if (constraints & ScreenConstraint) {
        appletConstraints |= ScreenConstraint;
    }

    if (appletConstraints != NoConstraint) {
        foreach (Applet *applet, applets) {
            applet->updateConstraints(appletConstraints);
        }
    }

    if (toolBox && (constraints & Plasma::SizeConstraint ||
                    constraints & Plasma::FormFactorConstraint ||
                    constraints & Plasma::ScreenConstraint ||
                    constraints & Plasma::StartupCompletedConstraint)) {
        //kDebug() << "Positioning toolbox";
        positionToolBox();
    }

    if (constraints & Plasma::StartupCompletedConstraint && type < Containment::CustomContainment) {
        q->addToolBoxAction(q->action("remove"));
        checkRemoveAction();
    }
}

Applet *ContainmentPrivate::addApplet(const QString &name, const QVariantList &args,
                                      const QRectF &appletGeometry, uint id, bool delayInit)
{
    if (!q->isContainment()) {
        return 0;
    }

    if (!delayInit && q->immutability() != Mutable) {
#ifndef NDEBUG
        kDebug() << "addApplet for" << name << "requested, but we're currently immutable!";
#endif
        return 0;
    }

    QGraphicsView *v = q->view();
    if (v) {
        v->setCursor(Qt::BusyCursor);
    }

    Applet *applet = PluginLoader::self()->loadApplet(name, id, args);
    if (v) {
        v->unsetCursor();
    }

    if (!applet) {
#ifndef NDEBUG
        kDebug() << "Applet" << name << "could not be loaded.";
#endif
        applet = new Applet(0, QString(), id);
        applet->setFailedToLaunch(true, i18n("Could not find requested component: %1", name));
    }

    //kDebug() << applet->name() << "sizehint:" << applet->sizeHint() << "geometry:" << applet->geometry();

    q->addApplet(applet, appletGeometry.topLeft(), delayInit);
    return applet;
}

bool ContainmentPrivate::regionIsEmpty(const QRectF &region, Applet *ignoredApplet) const
{
    foreach (Applet *applet, applets) {
        if (applet != ignoredApplet && applet->geometry().intersects(region)) {
            return false;
        }
    }
    return true;
}

void ContainmentPrivate::appletDeleted(Plasma::Applet *applet)
{
    applets.removeAll(applet);
    if (focusedApplet == applet) {
        focusedApplet = 0;
    }

    emit q->appletRemoved(applet);
    emit q->configNeedsSaving();
}

void ContainmentPrivate::appletAppeared(Applet *applet)
{
    //kDebug() << type << Containment::DesktopContainment;
    KConfigGroup *cg = applet->d->mainConfigGroup();
    applet->save(*cg);
    emit q->configNeedsSaving();
}

bool ContainmentPrivate::isPanelContainment() const
{
    return type == Containment::PanelContainment || type == Containment::CustomPanelContainment;
}

bool ContainmentPrivate::prepareContainmentActions(const QString &trigger, const QPoint &screenPos, KMenu *menu)
{
    ContainmentActions *plugin = actionPlugins()->value(trigger);
    if (!plugin) {
        return false;
    }
    plugin->setContainment(q);

    if (!plugin->isInitialized()) {
        KConfigGroup cfg = q->containmentActionsConfig();
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }

    if (plugin->configurationRequired()) {
        KMenu *localMenu = menu ? menu : new KMenu();

        localMenu->addTitle(i18n("This plugin needs to be configured"));
        localMenu->addAction(q->action("configure"));

        if (!menu) {
            localMenu->exec(screenPos);
            delete localMenu;
        }

        return false;
    } else if (menu) {
        QList<QAction*> actions = plugin->contextualActions();
        if (actions.isEmpty()) {
            //it probably didn't bother implementing the function. give the user a chance to set
            //a better plugin.  note that if the user sets no-plugin this won't happen...
            if (!isPanelContainment() && q->action("configure")) {
                menu->addAction(q->action("configure"));
            }
        } else {
            menu->addActions(actions);
        }
    }

    return true;
}

QHash<QString, ContainmentActions*> * ContainmentPrivate::actionPlugins()
{
    switch (containmentActionsSource) {
        case Activity:
            //FIXME
        case Local:
            return &localActionPlugins;
        default:
            return &globalActionPlugins;
    }
}

}
