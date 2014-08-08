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

#include <QAction>
#include <QHash>
#include <QFile>
#include <QDebug>
#include <QMimeDatabase>

#include <qstandardpaths.h>
#include <klocalizedstring.h>
#include <kactioncollection.h>
#include <kfileitemactions.h>

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
    KService::Ptr service = KService::serviceByDesktopName(application);
    if (service || !QStandardPaths::findExecutable(application).isNull() || QFile::exists(application)) {
        d->applicationNames[applet] = application;
        if (!d->urlLists.contains(applet)) {
            connect(applet, SIGNAL(destroyed(QObject*)), this, SLOT(cleanupApplet(QObject*)));
        } else {
            QAction *a = applet->actions()->action("run associated application");
            if (a) {
                a->setIcon(QIcon::fromTheme("system-run"));
                a->setText(i18n("Run the Associated Application"));
            }
        }
    }
}

QString AssociatedApplicationManager::application(const Plasma::Applet *applet) const
{
    return d->applicationNames.value(applet);
}

void AssociatedApplicationManager::setUrls(Plasma::Applet *applet, const QList<QUrl> &urls)
{
    d->urlLists[applet] = urls;
    if (!d->applicationNames.contains(applet)) {
        QAction *a = applet->actions()->action("run associated application");
        if (a) {
            QMimeDatabase mimeDb;
            KService::List apps = KFileItemActions::associatedApplications(mimeDb.mimeTypeForUrl(urls.first()).aliases(), QString());
            if (!apps.isEmpty()) {
                a->setIcon(QIcon::fromTheme(apps.first()->icon()));
                a->setText(i18n("Open in %1", apps.first()->genericName()));
            }
        }
    }
}

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
            qWarning() << "couldn't run" << d->applicationNames.value(applet) << d->urlLists.value(applet);
        }
#else
        QString execCommand = d->applicationNames.value(applet);

        // Clean-up the %u and friends from the exec command (KRun expect them, not QProcess)
        execCommand = execCommand.replace(QRegExp("%[a-z]"), QString());
        execCommand = execCommand.trimmed();

        QStringList parameters = d->urlLists.value(applet).toStringList();
        QProcess::startDetached(execCommand, parameters);
#endif

    } else if (d->urlLists.contains(applet) && !d->urlLists.value(applet).isEmpty()) {
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

