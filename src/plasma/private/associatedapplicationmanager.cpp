/*
    SPDX-FileCopyrightText: 2009 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "associatedapplicationmanager_p.h"

#include "config-plasma.h"
#include "debug_p.h"

#include <KSycoca>
#include <QAction>
#include <QDebug>
#include <QFile>
#include <QHash>
#include <QMimeDatabase>

#include <KActionCollection>
#include <KApplicationTrader>
#include <KIO/ApplicationLauncherJob>
#include <KIO/JobUiDelegateFactory>
#include <KIO/OpenUrlJob>
#include <KLocalizedString>
#include <KNotificationJobUiDelegate>
#include <QStandardPaths>

#include "plasma/applet.h"

namespace Plasma
{
class AssociatedApplicationManagerPrivate
{
public:
    AssociatedApplicationManagerPrivate()
    {
    }

    ~AssociatedApplicationManagerPrivate()
    {
    }

    void cleanupApplet(QObject *obj)
    {
        Plasma::Applet *applet = static_cast<Plasma::Applet *>(obj);
        applicationNames.remove(applet);
        urlLists.remove(applet);
    }

    void updateActionNames()
    {
        QMimeDatabase mimeDb;

        QHash<const Plasma::Applet *, QList<QUrl>>::iterator i;
        for (i = urlLists.begin(); i != urlLists.end(); ++i) {
            QAction *a = i.key()->actions()->action(QStringLiteral("run associated application"));
            if (a) {
                const QString mimeType = mimeDb.mimeTypeForUrl(i.value().first()).name();
                const KService::List apps = KApplicationTrader::queryByMimeType(mimeType);
                if (!apps.isEmpty()) {
                    a->setIcon(QIcon::fromTheme(apps.first()->icon()));
                    a->setText(i18n("Open with %1", apps.first()->genericName().isEmpty() ? apps.first()->genericName() : apps.first()->name()));
                } else {
                    a->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
                    a->setText(i18n("Run the Associated Application"));
                }
            }
        }
    }

    QHash<const Plasma::Applet *, QString> applicationNames;
    QHash<const Plasma::Applet *, QList<QUrl>> urlLists;
};

class AssociatedApplicationManagerSingleton
{
public:
    AssociatedApplicationManager self;
};

Q_GLOBAL_STATIC(AssociatedApplicationManagerSingleton, privateAssociatedApplicationManagerSelf)

AssociatedApplicationManager::AssociatedApplicationManager(QObject *parent)
    : QObject(parent)
    , d(new AssociatedApplicationManagerPrivate())
{
    connect(KSycoca::self(), SIGNAL(databaseChanged()), this, SLOT(updateActionNames()));
}

AssociatedApplicationManager::~AssociatedApplicationManager()
{
    delete d;
}

AssociatedApplicationManager *AssociatedApplicationManager::self()
{
    return &privateAssociatedApplicationManagerSelf()->self;
}

void AssociatedApplicationManager::setApplication(Plasma::Applet *applet, const QString &application)
{
    const bool hasAppBefore = d->applicationNames.contains(applet);
    const bool hasUrls = d->urlLists.contains(applet);

    // unsetting the application?
    if (application.isEmpty()) {
        if (hasAppBefore) {
            d->applicationNames.remove(applet);
            if (!hasUrls) {
                disconnect(applet, SIGNAL(destroyed(QObject *)), this, SLOT(cleanupApplet(QObject *)));
            }
        }
        return;
    }

    KService::Ptr service = KService::serviceByDesktopName(application);
    if (service || !QStandardPaths::findExecutable(application).isNull() || QFile::exists(application)) {
        d->applicationNames[applet] = application;

        if (hasUrls) {
            QAction *a = applet->actions()->action(QStringLiteral("run associated application"));
            if (a) {
                a->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
                a->setText(i18n("Run the Associated Application"));
            }
        } else if (!hasAppBefore) {
            connect(applet, SIGNAL(destroyed(QObject *)), this, SLOT(cleanupApplet(QObject *)));
        }
    }
}

QString AssociatedApplicationManager::application(const Plasma::Applet *applet) const
{
    return d->applicationNames.value(applet);
}

void AssociatedApplicationManager::setUrls(Plasma::Applet *applet, const QList<QUrl> &urls)
{
    const bool hasApp = d->applicationNames.contains(applet);
    const bool hasUrlsBefore = d->urlLists.contains(applet);

    // unsetting the urls?
    if (urls.isEmpty()) {
        if (hasUrlsBefore) {
            d->urlLists.remove(applet);
            if (!hasApp) {
                disconnect(applet, SIGNAL(destroyed(QObject *)), this, SLOT(cleanupApplet(QObject *)));
            }
        }
        return;
    }

    d->urlLists[applet] = urls;

    if (!hasApp) {
        QAction *a = applet->actions()->action(QStringLiteral("run associated application"));
        if (a) {
            QMimeDatabase mimeDb;
            const QString mimeType = mimeDb.mimeTypeForUrl(urls.first()).name();
            const KService::List apps = KApplicationTrader::queryByMimeType(mimeType);
            if (!apps.isEmpty()) {
                a->setIcon(QIcon::fromTheme(apps.first()->icon()));
                a->setText(i18n("Open with %1", apps.first()->genericName().isEmpty() ? apps.first()->genericName() : apps.first()->name()));
            } else {
                a->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
                a->setText(i18n("Run the Associated Application"));
            }
        }
        if (!hasUrlsBefore) {
            connect(applet, SIGNAL(destroyed(QObject *)), this, SLOT(cleanupApplet(QObject *)));
        }
    }
}
// TODO: updateAction slot, called on setting of url or app, and on sycoca change
QList<QUrl> AssociatedApplicationManager::urls(const Plasma::Applet *applet) const
{
    return d->urlLists.value(applet);
}

void AssociatedApplicationManager::run(Plasma::Applet *applet)
{
    if (d->applicationNames.contains(applet)) {
        const QString exec = d->applicationNames.value(applet);
        KService::Ptr service(new KService(exec, exec, QString() /*icon*/));
        KIO::ApplicationLauncherJob *job = new KIO::ApplicationLauncherJob(service);
        job->setUrls(d->urlLists.value(applet));
        job->setUiDelegate(new KNotificationJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled));
        connect(job, &KJob::result, this, [this, applet](KJob *job) {
            if (job->error()) {
                qCWarning(LOG_PLASMA) << "couldn't run" << d->applicationNames.value(applet) << d->urlLists.value(applet) << job->errorString();
            }
        });
        job->start();
    } else if (d->urlLists.contains(applet)) {
        KIO::OpenUrlJob *job = new KIO::OpenUrlJob(d->urlLists.value(applet).first());
        // TODO use a KNotifications-based ui delegate that supports the Open With dialog
        job->setUiDelegate(KIO::createDefaultJobUiDelegate(KJobUiDelegate::AutoHandlingEnabled, nullptr));
        job->start();
    }
}

bool AssociatedApplicationManager::appletHasValidAssociatedApplication(const Plasma::Applet *applet) const
{
    return (d->applicationNames.contains(applet) || d->urlLists.contains(applet));
}

} // namespace Plasma

#include <moc_associatedapplicationmanager_p.cpp>
