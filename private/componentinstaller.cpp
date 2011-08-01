/*
 *   Copyright 2011 Kevin Kofler <kevin.kofler@chello.at>
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

#include "private/componentinstaller_p.h"

#include <kglobal.h>

#ifdef PLASMA_ENABLE_PACKAGEKIT_SUPPORT
#include <QSet>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QWidget>
#include <QLatin1String>
#include <QStringList>
#endif

namespace Plasma
{

class ComponentInstallerPrivate
{
    public:
#ifdef PLASMA_ENABLE_PACKAGEKIT_SUPPORT
        QSet<QString> alreadyPrompted;
#endif
};

class ComponentInstallerSingleton
{
    public:
        ComponentInstaller self;
};

K_GLOBAL_STATIC(ComponentInstallerSingleton, privateComponentInstallerSelf)

ComponentInstaller *ComponentInstaller::self()
{
    return &privateComponentInstallerSelf->self;
}

ComponentInstaller::ComponentInstaller()
    : d(new ComponentInstallerPrivate)
{
}

ComponentInstaller::~ComponentInstaller()
{
    delete d;
}

void ComponentInstaller::installMissingComponent(const QString &type,
                                                 const QString &name,
                                                 QWidget *parent, bool force)
{
#ifdef PLASMA_ENABLE_PACKAGEKIT_SUPPORT
    QString searchString = type + '-' + name;

    if (!force) {
        if (d->alreadyPrompted.contains(searchString)) {
            return;
        }
    }

    d->alreadyPrompted.insert(searchString);

    QDBusInterface packageKit(QLatin1String("org.freedesktop.PackageKit"),
                              QLatin1String("/org/freedesktop/PackageKit"),
                              QLatin1String("org.freedesktop.PackageKit.Modify"));
    // We don't check packageKit.isValid() because the service is activated on
    // demand, so it will show up as "not valid".
    WId wid = 0;
    if (parent) {
        wid = parent->winId();
    }
    QStringList resources;
    resources.append(searchString);
    packageKit.asyncCall(QLatin1String("InstallResources"), (unsigned int) wid,
                         QLatin1String("plasma-service"), resources, QString());
#else
    Q_UNUSED(type);
    Q_UNUSED(name);
    Q_UNUSED(parent);
    Q_UNUSED(force);
#endif
}

} // namespace Plasma
