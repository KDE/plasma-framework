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
#include <QMimeData>
#include <QMimeDatabase>
#include <QDropEvent>
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

#include "containmentactions.h"
#include "containmentactionspluginsconfig.h"
#include "corona.h"
#include "pluginloader.h"
#include "svg.h"

#include "remote/accessappletjob.h"
#include "remote/accessmanager.h"

#include "private/applet_p.h"
#include "private/containmentactionspluginsconfig_p.h"


namespace Plasma
{


const char DeclarativeContainmentPrivate::defaultWallpaperMode[] = "SingleImage";


void DeclarativeContainmentPrivate::showDropZoneDelayed()
{
    dropZoneStarted = true;
    q->showDropZone(dropPoints.value(0).toPoint());
    dropPoints.remove(0);
}

void DeclarativeContainmentPrivate::dropData(QPoint screenPos, QDropEvent *dropEvent)
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
            QRectF geom(screenPos, QSize(0, 0));
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
                    dropPoints[job] = screenPos;
                }
                QObject::connect(AccessManager::self(), SIGNAL(finished(Plasma::AccessAppletJob*)),
                                 q, SLOT(remoteAppletReady(Plasma::AccessAppletJob*)));
            }
#if !PLASMA_NO_KIO
            else {
                QMimeDatabase db;
                QMimeType mime = db.mimeTypeForUrl(url);
                QString mimeName = mime.name();
                QRectF geom(screenPos, QSize());
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
                    dropPoints[job] = screenPos;
                }

                QObject::connect(job, SIGNAL(result(KJob*)), q, SLOT(dropJobResult(KJob*)));
                QObject::connect(job, SIGNAL(mimetype(KIO::Job*,QString)),
                                 q, SLOT(mimeTypeRetrieved(KIO::Job*,QString)));

                KMenu *choices = new KMenu("Content dropped");
                choices->addAction(KDE::icon("process-working"), i18n("Fetching file type..."));
                if (dropEvent) {
                    choices->popup(dropEvent->pos());
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

                QRectF geom(screenPos, QSize());
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

void DeclarativeContainmentPrivate::clearDataForMimeJob(KIO::Job *job)
{
#if !PLASMA_NO_KIO
    QObject::disconnect(job, 0, q, 0);
    dropPoints.remove(job);
    KMenu *choices = dropMenus.take(job);
    delete choices;
    job->kill();
#endif // PLASMA_NO_KIO
}

void DeclarativeContainmentPrivate::dropJobResult(KJob *job)
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

void DeclarativeContainmentPrivate::mimeTypeRetrieved(KIO::Job *job, const QString &mimeType)
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

void DeclarativeContainmentPrivate::focusApplet(Plasma::Applet *applet)
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

void DeclarativeContainmentPrivate::remoteAppletReady(Plasma::AccessAppletJob *job)
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


}
