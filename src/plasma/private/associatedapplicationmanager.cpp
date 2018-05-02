/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "associatedapplicationmanager_p.h"

#include "config-plasma.h"
#include "debug_p.h"

#include <QAction>
#include <QHash>
#include <QFile>
#include <QDebug>
#include <QMimeDatabase>
#include <KSycoca>

#include <qstandardpaths.h>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <KMimeTypeTrader>

#if !PLASMA_NO_KIO
#include <krun.h>
#else
#include <QProcess>
#include <QDesktopServices>
#endif

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

        QHash<const Plasma::Applet *, QList<QUrl> >::iterator i;
        for (i = urlLists.begin(); i != urlLists.end(); ++i) {
            QAction *a = i.key()->actions()->action(QStringLiteral("run associated application"));
            if (a) {
                const QString mimeType = mimeDb.mimeTypeForUrl(i.value().first()).name();
                const KService::List apps = KMimeTypeTrader::self()->query(mimeType);
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
    QHash<const Plasma::Applet *, QList<QUrl> > urlLists;
};

class AssociatedApplicationManagerSingleton
{
public:
    AssociatedApplicationManager self;
};

Q_GLOBAL_STATIC(AssociatedApplicationManagerSingleton, privateAssociatedApplicationManagerSelf)

AssociatedApplicationManager::AssociatedApplicationManager(QObject *parent)
    : QObject(parent),
      d(new AssociatedApplicationManagerPrivate())
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
                disconnect(applet, SIGNAL(destroyed(QObject*)), this, SLOT(cleanupApplet(QObject*)));
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
            connect(applet, SIGNAL(destroyed(QObject*)), this, SLOT(cleanupApplet(QObject*)));
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
                disconnect(applet, SIGNAL(destroyed(QObject*)), this, SLOT(cleanupApplet(QObject*)));
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
            const KService::List apps = KMimeTypeTrader::self()->query(mimeType);
            if (!apps.isEmpty()) {
                a->setIcon(QIcon::fromTheme(apps.first()->icon()));
                a->setText(i18n("Open with %1", apps.first()->genericName().isEmpty() ? apps.first()->genericName() : apps.first()->name()));
            } else {
                a->setIcon(QIcon::fromTheme(QStringLiteral("system-run")));
                a->setText(i18n("Run the Associated Application"));
            }
        }
        if (!hasUrlsBefore) {
            connect(applet, SIGNAL(destroyed(QObject*)), this, SLOT(cleanupApplet(QObject*)));
        }
    }
}
//TODO: updateAction slot, called on setting of url or app, and on sycoca change
QList<QUrl> AssociatedApplicationManager::urls(const Plasma::Applet *applet) const
{
    return d->urlLists.value(applet);
}

void AssociatedApplicationManager::run(Plasma::Applet *applet)
{
    if (d->applicationNames.contains(applet)) {
#if !PLASMA_NO_KIO
        bool success = KRun::run(d->applicationNames.value(applet), d->urlLists.value(applet), 0);
        if (!success) {
            qCWarning(LOG_PLASMA) << "couldn't run" << d->applicationNames.value(applet) << d->urlLists.value(applet);
        }
#else
        QString execCommand = d->applicationNames.value(applet);

        // Clean-up the %u and friends from the exec command (KRun expect them, not QProcess)
        execCommand = execCommand.replace(QRegExp("%[a-z]"), QString());
        execCommand = execCommand.trimmed();

        QStringList parameters = d->urlLists.value(applet).toStringList();
        QProcess::startDetached(execCommand, parameters);
#endif

    } else if (d->urlLists.contains(applet)) {
#if !PLASMA_NO_KIO
        KRun *krun = new KRun(d->urlLists.value(applet).first(), 0);
        krun->setAutoDelete(true);
#else
        QDesktopServices::openUrl(d->urlLists.value(applet).first());
#endif
    }
}

bool AssociatedApplicationManager::appletHasValidAssociatedApplication(const Plasma::Applet *applet) const
{
    return (d->applicationNames.contains(applet) || d->urlLists.contains(applet));
}

} // namespace Plasma

#include <moc_associatedapplicationmanager_p.cpp>

