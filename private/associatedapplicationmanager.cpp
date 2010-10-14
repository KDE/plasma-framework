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

#include <QHash>
#include <QFile>

#include <kstandarddirs.h>
#include <kicon.h>

#ifndef PLASMA_NO_KIO
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
    QHash<const Plasma::Applet *, KUrl::List> urlLists;
};

class AssociatedApplicationManagerSingleton
{
    public:
        AssociatedApplicationManager self;
};

K_GLOBAL_STATIC(AssociatedApplicationManagerSingleton, privateAssociatedApplicationManagerSelf)


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
    return &privateAssociatedApplicationManagerSelf->self;
}

void AssociatedApplicationManager::setApplication(Plasma::Applet *applet, const QString &application)
{
    KService::Ptr service = KService::serviceByDesktopName(application);
    if (service || !KStandardDirs::findExe(application).isNull() || QFile::exists(application)) {
        d->applicationNames[applet] = application;
        if (!d->urlLists.contains(applet)) {
            connect(applet, SIGNAL(destroyed(QObject *)), this, SLOT(cleanupApplet(QObject *)));
        }
    }
}

QString AssociatedApplicationManager::application(const Plasma::Applet *applet) const
{
    return d->applicationNames.value(applet);
    if (!d->applicationNames.contains(applet)) {
            connect(applet, SIGNAL(destroyed(QObject *)), this, SLOT(cleanupApplet(QObject *)));
        }
}

void AssociatedApplicationManager::setUrls(Plasma::Applet *applet, const KUrl::List &urls)
{
    d->urlLists[applet] = urls;
}

KUrl::List AssociatedApplicationManager::urls(const Plasma::Applet *applet) const
{
    return d->urlLists.value(applet);
}

void AssociatedApplicationManager::run(Plasma::Applet *applet)
{
    if (d->applicationNames.contains(applet)) {
#ifndef PLASMA_NO_KIO
        bool success = KRun::run(d->applicationNames.value(applet), d->urlLists.value(applet), 0);
#else
        QString execCommand = d->applicationNames.value(applet);

        // Clean-up the %u and friends from the exec command (KRun expect them, not QProcess)
        execCommand = execCommand.replace(QRegExp("%[a-z]"), QString());
        execCommand = execCommand.trimmed();

        QStringList parameters = d->urlLists.value(applet).toStringList();
        bool success = QProcess::startDetached(execCommand, parameters);
#endif

        if (!success) {
            applet->showMessage(KIcon("application-exit"), i18n("There was an error attempting to exec the associated application with this widget."), ButtonOk);
        }

    } else if (d->urlLists.contains(applet) && !d->urlLists.value(applet).isEmpty()) {
#ifndef PLASMA_NO_KIO
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

