/*
 *   Copyright 2008 Chani Armitage <chani@kde.org>
 *   Copyright 2008, 2009 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Marco Martin <mart@kde.org>
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

#include "containmentinterface.h"
#include "wallpaperinterface.h"
#include <kdeclarative/qmlobject.h>

#include <QClipboard>
#include <QQmlExpression>
#include <QQmlProperty>
#include <QMimeData>

#include <KActionCollection>
#include <KAuthorized>
#include <QDebug>
#include <KLocalizedString>
#include <kurlmimedata.h>
#include <KMimeType>
#include <KTemporaryFile>

#ifndef PLASMA_NO_KIO
#include "kio/jobclasses.h" // for KIO::JobFlags
#include "kio/job.h"
#include "kio/scheduler.h"
#endif

#include <plasma.h>
#include <Plasma/ContainmentActions>
#include <Plasma/Corona>
#include <Plasma/Package>
#include <Plasma/PluginLoader>

#include "kdeclarative/configpropertymap.h"

ContainmentInterface::ContainmentInterface(DeclarativeAppletScript *parent)
    : AppletInterface(parent),
      m_wallpaperInterface(0)
{
    setAcceptedMouseButtons(Qt::AllButtons);

    qmlRegisterType<ContainmentInterface>();

    connect(containment(), &Plasma::Containment::appletRemoved,
            this, &ContainmentInterface::appletRemovedForward);
    connect(containment(), &Plasma::Containment::appletAdded,
            this, &ContainmentInterface::appletAddedForward);
    connect(containment(), &Plasma::Containment::activityChanged,
            this, &ContainmentInterface::activityChanged);
    connect(containment(), &Plasma::Containment::wallpaperChanged,
            this, &ContainmentInterface::loadWallpaper);
    connect(containment(), &Plasma::Containment::drawWallpaperChanged,
            this, &ContainmentInterface::drawWallpaperChanged);
    connect(containment(), &Plasma::Containment::containmentTypeChanged,
            this, &ContainmentInterface::containmentTypeChanged);

     if (containment()->corona()) {
         connect(containment()->corona(), &Plasma::Corona::availableScreenRegionChanged,
                 this, &ContainmentInterface::availableScreenRegionChanged);
     }

    if (!m_appletInterfaces.isEmpty()) {
        emit appletsChanged();
    }
}

void ContainmentInterface::init()
{

    AppletInterface::init();

    if (m_qmlObject->rootObject()->property("minimumWidth").isValid()) {
        connect(m_qmlObject->rootObject(), SIGNAL(minimumWidthChanged()),
                this, SIGNAL(minimumWidthChanged()));
    }
    if (m_qmlObject->rootObject()->property("minimumHeight").isValid()) {
        connect(m_qmlObject->rootObject(), SIGNAL(minimumHeightChanged()),
                this, SIGNAL(minimumHeightChanged()));
    }

    if (m_qmlObject->rootObject()->property("maximumWidth").isValid()) {
        connect(m_qmlObject->rootObject(), SIGNAL(maximumWidthChanged()),
                this, SIGNAL(maximumWidthChanged()));
    }
    if (m_qmlObject->rootObject()->property("maximumHeight").isValid()) {
        connect(m_qmlObject->rootObject(), SIGNAL(maximumHeightChanged()),
                this, SIGNAL(maximumHeightChanged()));
    }

    if (m_qmlObject->rootObject()->property("implicitWidth").isValid()) {
        connect(m_qmlObject->rootObject(), SIGNAL(implicitWidthChanged()),
                this, SIGNAL(implicitWidthChanged()));
    }
    if (m_qmlObject->rootObject()->property("implicitHeight").isValid()) {
        connect(m_qmlObject->rootObject(), SIGNAL(implicitHeightChanged()),
                this, SIGNAL(implicitHeightChanged()));
    }

    emit fillWidthChanged();
    emit fillHeightChanged();
    emit minimumWidthChanged();
    emit minimumHeightChanged();
    emit implicitWidthChanged();
    emit implicitHeightChanged();
    emit maximumWidthChanged();
    emit maximumHeightChanged();
}

QList <QObject *> ContainmentInterface::applets()
{
    return m_appletInterfaces;
}

void ContainmentInterface::setDrawWallpaper(bool drawWallpaper)
{
    if (drawWallpaper == m_appletScriptEngine->drawWallpaper()) {
        return;
    }

    m_appletScriptEngine->setDrawWallpaper(drawWallpaper);

    loadWallpaper();
}

bool ContainmentInterface::drawWallpaper()
{
    return m_appletScriptEngine->drawWallpaper();
}

Plasma::Types::ContainmentType ContainmentInterface::containmentType() const
{
    return m_appletScriptEngine->containmentType();
}

void ContainmentInterface::setContainmentType(Plasma::Types::ContainmentType type)
{
    m_appletScriptEngine->setContainmentType(type);
}

void ContainmentInterface::lockWidgets(bool locked)
{
    containment()->setImmutability(!locked ? Plasma::Types::Mutable : Plasma::Types::UserImmutable);
    emit immutableChanged();
}

QRectF ContainmentInterface::screenGeometry(int id) const
{
    QRectF rect;
    if (containment()->corona()) {
        rect = QRectF(containment()->corona()->screenGeometry(id));
    }

    return rect;
}

QVariantList ContainmentInterface::availableScreenRegion(int id) const
{
    QRegion reg;
    if (containment()->corona()) {
        reg = containment()->corona()->availableScreenRegion(id);
    }

    QVariantList regVal;
    foreach (QRect rect, reg.rects()) {
        regVal << QVariant::fromValue(QRectF(rect));
    }
    return regVal;
}

Plasma::Applet *ContainmentInterface::addApplet(const QString &plugin, const QVariantList &args, const QPoint &pos)
{
    //HACK
    //This is necessary to delay the appletAdded signal (of containmentInterface) AFTER the applet graphics object has been created
    blockSignals(true);
    Plasma::Applet *applet = containment()->createApplet(plugin, args);

    QObject *appletGraphicObject;
    if (applet) {
        appletGraphicObject = applet->property("graphicObject").value<QObject *>();
    }

    blockSignals(false);

    emit appletAdded(appletGraphicObject, pos.x(), pos.y());
    emit appletsChanged();
    return applet;
}

void ContainmentInterface::setAppletArgs(Plasma::Applet *applet, const QString &mimetype, const QString &data)
{
    if (!applet) {
        return;
    }

    AppletInterface *appletInterface = applet->property("graphicObject").value<AppletInterface *>();
    if (appletInterface) {
        emit appletInterface->externalData(mimetype, data);
    }
}

void ContainmentInterface::processMimeData(QMimeData *mimeData, int x, int y)
{
    if (!mimeData) {
        return;
    }

    //const QMimeData *mimeData = data;

    qDebug() << "Arrived mimeData" << mimeData->urls() << mimeData->formats() << "at" << x << ", " << y;

    if (mimeData->hasFormat("text/x-plasmoidservicename")) {
        QString data = mimeData->data("text/x-plasmoidservicename");
        const QStringList appletNames = data.split('\n', QString::SkipEmptyParts);
        foreach (const QString &appletName, appletNames) {
            qDebug() << "adding" << appletName;
            addApplet(appletName, QVariantList(), QPoint(x, y));
        }
    } else if (mimeData->hasUrls()) {
        //TODO: collect the mimetypes of available script engines and offer
        //      to create widgets out of the matching URLs, if any
        const QList<QUrl> urls = KUrlMimeData::urlsFromMimeData(mimeData);
        foreach (const QUrl &url, urls) {

#ifndef PLASMA_NO_KIO
            KMimeType::Ptr mime = KMimeType::findByUrl(url);
            QString mimeName = mime->name();
            QVariantList args;
            args << url.url();
            qDebug() << "can decode" << mimeName << args;

            // It may be a directory or a file, let's stat
            KIO::JobFlags flags = KIO::HideProgressInfo;
            KIO::MimetypeJob *job = KIO::mimetype(url, flags);
            m_dropPoints[job] = QPoint(x, y);


            QObject::connect(job, SIGNAL(result(KJob*)), this, SLOT(dropJobResult(KJob*)));
            QObject::connect(job, SIGNAL(mimetype(KIO::Job*,QString)),
                                this, SLOT(mimeTypeRetrieved(KIO::Job*,QString)));

            QMenu *choices = new QMenu("Content dropped");
            choices->addAction(QIcon::fromTheme("process-working"), i18n("Fetching file type..."));
            choices->popup(window() ? window()->mapToGlobal(QPoint(x, y)) : QPoint(x, y));


            m_dropMenus[job] = choices;
#endif
        }

    } else {
        QStringList formats = mimeData->formats();
        QHash<QString, KPluginInfo> seenPlugins;
        QHash<QString, QString> pluginFormats;

        foreach (const QString &format, formats) {
            KPluginInfo::List plugins = Plasma::PluginLoader::self()->listAppletInfoForMimeType(format);

            foreach (const KPluginInfo &plugin, plugins) {
                if (seenPlugins.contains(plugin.pluginName())) {
                    continue;
                }

                seenPlugins.insert(plugin.pluginName(), plugin);
                pluginFormats.insert(plugin.pluginName(), format);
            }
        }
        //qDebug() << "Mimetype ..." << formats << seenPlugins.keys() << pluginFormats.values();

        QString selectedPlugin;

        if (seenPlugins.isEmpty()) {
            // do nothing
        } else if (seenPlugins.count() == 1) {
            selectedPlugin = seenPlugins.constBegin().key();
        } else {
            QMenu choices;
            QHash<QAction *, QString> actionsToPlugins;
            foreach (const KPluginInfo &info, seenPlugins) {
                QAction *action;
                if (!info.icon().isEmpty()) {
                    action = choices.addAction(QIcon::fromTheme(info.icon()), info.name());
                } else {
                    action = choices.addAction(info.name());
                }

                actionsToPlugins.insert(action, info.pluginName());
            }

            QAction *choice = choices.exec(window() ? window()->mapToGlobal(QPoint(x, y)) : QPoint(x, y));
            if (choice) {
                selectedPlugin = actionsToPlugins[choice];
            }
        }

        if (!selectedPlugin.isEmpty()) {

            //TODO: remove all this temp file logic
            KTemporaryFile tempFile;
            if (mimeData && tempFile.open()) {
                //TODO: what should we do with files after the applet is done with them??
                tempFile.setAutoRemove(false);

                {
                    QDataStream stream(&tempFile);
                    QByteArray data = mimeData->data(pluginFormats[selectedPlugin]);
                    stream.writeRawData(data, data.size());
                }

                QVariantList args;
                args << tempFile.fileName();
                qDebug() << args;
                tempFile.close();

                Plasma::Applet *applet = addApplet(selectedPlugin, args, QPoint(x, y));
                setAppletArgs(applet, pluginFormats[selectedPlugin], mimeData->data(pluginFormats[selectedPlugin]));
            }
        }
    }
}

void ContainmentInterface::clearDataForMimeJob(KIO::Job *job)
{
#ifndef PLASMA_NO_KIO
    QObject::disconnect(job, 0, this, 0);
    m_dropPoints.remove(job);
    QMenu *choices = m_dropMenus.take(job);
    delete choices;
    job->kill();
#endif // PLASMA_NO_KIO
}

void ContainmentInterface::dropJobResult(KJob *job)
{
#ifndef PLASMA_NO_KIO
    KIO::TransferJob* tjob = dynamic_cast<KIO::TransferJob*>(job);
    if (!tjob) {
        qDebug() << "job is not a KIO::TransferJob, won't handle the drop...";
        clearDataForMimeJob(tjob);
        return;
    }
    if (job->error()) {
        qDebug() << "ERROR" << tjob->error() << ' ' << tjob->errorString();
    }
    // We call mimetypeRetrieved since there might be other mechanisms
    // for finding suitable applets. Cleanup happens there as well.
    mimeTypeRetrieved(qobject_cast<KIO::Job *>(job), QString());
#endif // PLASMA_NO_KIO
}

void ContainmentInterface::mimeTypeRetrieved(KIO::Job *job, const QString &mimetype)
{
#ifndef PLASMA_NO_KIO
    qDebug() << "Mimetype Job returns." << mimetype;
    KIO::TransferJob* tjob = dynamic_cast<KIO::TransferJob*>(job);
    if (!tjob) {
        qDebug() << "job should be a TransferJob, but isn't";
        clearDataForMimeJob(job);
        return;
    }
    KPluginInfo::List appletList = Plasma::PluginLoader::self()->listAppletInfoForUrl(tjob->url());
    if (mimetype.isEmpty() && !appletList.count()) {
        clearDataForMimeJob(job);
        qDebug() << "No applets found matching the url (" << tjob->url() << ") or the mimetype (" << mimetype << ")";
        return;
    } else {

        QPoint posi; // will be overwritten with the event's position
        if (m_dropPoints.keys().contains(tjob)) {
            posi = m_dropPoints[tjob];
            qDebug() << "Received a suitable dropEvent at" << posi;
        } else {
            qDebug() << "Bailing out. Cannot find associated dropEvent related to the TransferJob";
            clearDataForMimeJob(job);
            return;
        }

        QMenu *choices = m_dropMenus.value(tjob);
        if (!choices) {
            qDebug() << "Bailing out. No QMenu found for this job.";
            clearDataForMimeJob(job);
            return;
        }

        //TODO: remove assignment of applet args
        QVariantList args;
        args << tjob->url().url() << mimetype;

        qDebug() << "Creating menu for:" << mimetype  << posi << args;

        appletList << Plasma::PluginLoader::self()->listAppletInfoForMimeType(mimetype);
        KPluginInfo::List wallpaperList;
        //TODO: how restore wallpaper dnd?

        if (drawWallpaper()) {
            if (m_wallpaperInterface && m_wallpaperInterface->supportsMimetype(mimetype)) {
                wallpaperList << m_wallpaperInterface->package().metadata();
            } else {
                wallpaperList = WallpaperInterface::listWallpaperInfoForMimetype(mimetype);
            }
        }

        if (!appletList.isEmpty() || !wallpaperList.isEmpty()) {
            choices->clear();
            QHash<QAction *, QString> actionsToApplets;
            choices->addSection(i18n("Widgets"));
            foreach (const KPluginInfo &info, appletList) {
                qDebug() << info.name();
                QAction *action;
                if (!info.icon().isEmpty()) {
                    action = choices->addAction(QIcon::fromTheme(info.icon()), info.name());
                } else {
                    action = choices->addAction(info.name());
                }

                actionsToApplets.insert(action, info.pluginName());
                qDebug() << info.pluginName();
            }
            actionsToApplets.insert(choices->addAction(i18n("Icon")), "org.kde.icon");

            QHash<QAction *, QString> actionsToWallpapers;
            if (!wallpaperList.isEmpty())  {
                choices->addSection(i18n("Wallpaper"));

                QMap<QString, KPluginInfo> sorted;
                foreach (const KPluginInfo &info, appletList) {
                    sorted.insert(info.name(), info);
                }

                foreach (const KPluginInfo &info, wallpaperList) {
                    QAction *action;
                    if (!info.icon().isEmpty()) {
                        action = choices->addAction(QIcon::fromTheme(info.icon()), info.name());
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
                if (!mimetype.isEmpty() && !tjob->error()) {
                    tjob->putOnHold();
                    KIO::Scheduler::publishSlaveOnHold();
                }
                QString plugin = actionsToApplets.value(choice);
                if (plugin.isEmpty()) {
                    //set wallpapery stuff
                    plugin = actionsToWallpapers.value(choice);
                    if (m_wallpaperInterface && tjob->url().isValid()) {
                        m_wallpaperInterface->setUrl(tjob->url());
                    }
                } else {
                    Plasma::Applet *applet = addApplet(actionsToApplets[choice], args, posi);
                    setAppletArgs(applet, mimetype, tjob->url().toString());
                }

                clearDataForMimeJob(job);
                return;
            }
        } else {
            // we can at least create an icon as a link to the URL
            Plasma::Applet *applet = addApplet("org.kde.icon", args, posi);
            setAppletArgs(applet, mimetype, tjob->url().toString());
        }
    }

    clearDataForMimeJob(job);
#endif // PLASMA_NO_KIO
}



void ContainmentInterface::appletAddedForward(Plasma::Applet *applet)
{
    if (!applet) {
        return;
    }

    QObject *appletGraphicObject = applet->property("graphicObject").value<QObject *>();
    QObject *contGraphicObject = containment()->property("graphicObject").value<QObject *>();

    qDebug() << "Applet added on containment:" << containment()->title() << contGraphicObject
             << "Applet: " << applet << applet->title() << appletGraphicObject;

    if (contGraphicObject && appletGraphicObject) {
        appletGraphicObject->setProperty("visible", false);
        appletGraphicObject->setProperty("parent", QVariant::fromValue(contGraphicObject));

    //if an appletGraphicObject is not set, we have to display some error message
    } else if (contGraphicObject) {
        QObject *errorUi = qmlObject()->createObjectFromSource(QUrl::fromLocalFile(containment()->corona()->package().filePath("appleterror")));

        if (errorUi) {
            errorUi->setProperty("visible", false);
            errorUi->setProperty("parent", QVariant::fromValue(contGraphicObject));
            errorUi->setProperty("reason", applet->launchErrorMessage());
            appletGraphicObject = errorUi;
        }
    }

    m_appletInterfaces << appletGraphicObject;
    emit appletAdded(appletGraphicObject, 0, 0);
    emit appletsChanged();
}

void ContainmentInterface::appletRemovedForward(Plasma::Applet *applet)
{
    QObject *appletGraphicObject = applet->property("graphicObject").value<QObject *>();
    m_appletInterfaces.removeAll(appletGraphicObject);
    emit appletRemoved(appletGraphicObject);
    emit appletsChanged();
}

void ContainmentInterface::loadWallpaper()
{
    if (m_appletScriptEngine->drawWallpaper() && !containment()->wallpaper().isEmpty()) {
        delete m_wallpaperInterface;

        m_wallpaperInterface = new WallpaperInterface(this);
        m_wallpaperInterface->setZ(-1000);
        //Qml seems happier if the parent gets set in this way
        m_wallpaperInterface->setProperty("parent", QVariant::fromValue(this));

        //set anchors
        QQmlExpression expr(qmlObject()->engine()->rootContext(), m_wallpaperInterface, "parent");
        QQmlProperty prop(m_wallpaperInterface, "anchors.fill");
        prop.write(expr.evaluate());

        containment()->setProperty("wallpaperGraphicsObject", QVariant::fromValue(m_wallpaperInterface));
    } else {
        if (m_wallpaperInterface) {
            m_wallpaperInterface->deleteLater();
            m_wallpaperInterface = 0;
        }
    }
}

QString ContainmentInterface::activity() const
{
    return containment()->activity();
}

QList<QObject*> ContainmentInterface::actions() const
{
    //FIXME: giving directly a QList<QAction*> crashes

    //use a multimap to sort by action type
    QMultiMap<int, QObject*> actions;
    foreach (QAction *a, containment()->actions()->actions()) {
        if (a->isEnabled()) {
            actions.insert(a->data().toInt(), a);
        }
    }
    foreach (QAction *a, containment()->corona()->actions()->actions()) {
        if (a->isEnabled()) {
            actions.insert(a->data().toInt(), a);
        }
    }
    return actions.values();
}




//PROTECTED--------------------

void ContainmentInterface::mousePressEvent(QMouseEvent *event)
{
    event->setAccepted(containment()->containmentActions().contains(Plasma::ContainmentActions::eventToString(event)));
}

void ContainmentInterface::mouseReleaseEvent(QMouseEvent *event)
{
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    Plasma::ContainmentActions *plugin = containment()->containmentActions().value(trigger);

    if (!plugin || plugin->contextualActions().isEmpty()) {
        event->setAccepted(false);
        return;
    }


    //the plugin can be a single action or a context menu
    //Don't have an action list? execute as single action
    //and set the event position as action data
    if (plugin->contextualActions().length() == 1) {
        QAction *action = plugin->contextualActions().first();
        action->setData(event->pos());
        action->trigger();
        event->accept();
        return;
    }

    //FIXME: very inefficient appletAt() implementation
    Plasma::Applet *applet = 0;
    foreach (QObject *appletObject, m_appletInterfaces) {
        if (AppletInterface *ai = qobject_cast<AppletInterface *>(appletObject)) {
            if (ai->contains(ai->mapFromItem(this, event->posF()))) {
                applet = ai->applet();
                break;
            } else {
                ai = 0;
            }
        }
    }
    qDebug() << "Invoking menu for applet" << applet;

    QMenu desktopMenu;

    if (applet) {
        addAppletActions(desktopMenu, applet, event);
    } else {
        addContainmentActions(desktopMenu, event);
    }
    desktopMenu.exec(event->globalPos());
    event->accept();
}

void ContainmentInterface::wheelEvent(QWheelEvent *event)
{
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    Plasma::ContainmentActions *plugin = containment()->containmentActions().value(trigger);

    if (plugin) {
        if (event->delta() < 0) {
            plugin->performNextAction();
        } else {
            plugin->performPreviousAction();
        }
    } else {
        event->setAccepted(false);
    }
}



void ContainmentInterface::addAppletActions(QMenu &desktopMenu, Plasma::Applet *applet, QEvent *event)
{
    foreach (QAction *action, applet->contextualActions()) {
        if (action) {
            desktopMenu.addAction(action);
        }
    }

    if (!applet->failedToLaunch()) {
        QAction *configureApplet = applet->actions()->action("configure");
        if (configureApplet && configureApplet->isEnabled()) {
            desktopMenu.addAction(configureApplet);
        }

        QAction *runAssociatedApplication = applet->actions()->action("run associated application");
        if (runAssociatedApplication && runAssociatedApplication->isEnabled()) {
            desktopMenu.addAction(runAssociatedApplication);
        }
    }

    QMenu *containmentMenu = new QMenu(i18nc("%1 is the name of the containment", "%1 Options", containment()->title()), &desktopMenu);
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

    if (containment()->immutability() == Plasma::Types::Mutable) {
        QAction *closeApplet = applet->actions()->action("remove");
        //qDebug() << "checking for removal" << closeApplet;
        if (closeApplet) {
            if (!desktopMenu.isEmpty()) {
                desktopMenu.addSeparator();
            }

            //qDebug() << "adding close action" << closeApplet->isEnabled() << closeApplet->isVisible();
            desktopMenu.addAction(closeApplet);
        }
    }
}

void ContainmentInterface::addContainmentActions(QMenu &desktopMenu, QEvent *event)
{
    if (containment()->corona()->immutability() != Plasma::Types::Mutable &&
        !KAuthorized::authorizeKAction("plasma/containment_actions")) {
        //qDebug() << "immutability";
        return;
    }

    //this is what ContainmentPrivate::prepareContainmentActions was
    const QString trigger = Plasma::ContainmentActions::eventToString(event);
    Plasma::ContainmentActions *plugin = containment()->containmentActions().value(trigger);

    if (!plugin) {
        return;
    }

    if (plugin->containment() != containment()) {
        plugin->setContainment(containment());

        // now configure it
        KConfigGroup cfg(containment()->corona()->config(), "ActionPlugins");
        cfg = KConfigGroup(&cfg, QString::number(containment()->containmentType()));
        KConfigGroup pluginConfig = KConfigGroup(&cfg, trigger);
        plugin->restore(pluginConfig);
    }


    QList<QAction*> actions = plugin->contextualActions();

    if (actions.isEmpty()) {
        //it probably didn't bother implementing the function. give the user a chance to set
        //a better plugin.  note that if the user sets no-plugin this won't happen...
        if ((containment()->containmentType() != Plasma::Types::PanelContainment &&
             containment()->containmentType() != Plasma::Types::CustomPanelContainment) &&
            containment()->actions()->action("configure")) {
            desktopMenu.addAction(containment()->actions()->action("configure"));
        }
    } else {
        desktopMenu.addActions(actions);
    }


    return;
}

#include "moc_containmentinterface.cpp"
