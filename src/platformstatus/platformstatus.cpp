/*
 *   Copyright (C) 2013 by Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2013 by Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <platformstatus.h>

#include <QDBusConnection>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KDebug>
#include <KDirWatch>
#include <KPluginFactory>

#include "platformstatusadaptor.h"

const char *defaultPackage = "org.kde.desktop";

K_PLUGIN_FACTORY(PlatformStatusFactory, registerPlugin<PlatformStatus>();)
K_EXPORT_PLUGIN(PlatformStatusFactory("platformstatus"))

PlatformStatus::PlatformStatus(QObject *parent, const QVariantList &)
    : KDEDModule(parent)
{
    new PlatformStatusAdaptor(this);
    QDBusConnection::sessionBus().registerObject("/PlatformStatus", this);

    findShellPackage(false);

    const QString globalrcPath = QStandardPaths::locate(QStandardPaths::ConfigLocation, "kdeglobals");
    connect(KDirWatch::self(), SIGNAL(dirty(QString)), this, SLOT(fileDirtied(QString)));
    KDirWatch::self()->addFile(globalrcPath);
}

void PlatformStatus::findShellPackage(bool sendSignal)
{
    KConfigGroup group(KSharedConfig::openConfig("kdeglobals"), "DesktopShell");
    const QString package = group.readEntry("shellPackage", defaultPackage);

    const QString path = QStandardPaths::locate(QStandardPaths::GenericDataLocation,
                                                "plasma/shells/" + package + '/',
                                                QStandardPaths::LocateDirectory);
    if (path.isEmpty()) {
        if (package != defaultPackage) {
            group.deleteEntry("ShellPackage");
            findShellPackage(sendSignal);
        }

        return;
    }

    m_shellPackage = package;

    QString runtimePlatform = group.readEntry("RuntimePlatform", QString());
    KConfig packageDefaults(path + "contents/defaults", KConfig::SimpleConfig);
    group = KConfigGroup(&packageDefaults, "Desktop");
    runtimePlatform = group.readEntry("RuntimePlatform", runtimePlatform);
    const bool runtimeChanged = runtimePlatform != m_runtimePlatform.join(',');
    if (runtimeChanged) {
        m_runtimePlatform = runtimePlatform.split(',');
    }

    if (sendSignal) {
        emit shellPackageChanged(m_shellPackage);
        emit runtimePlatformChanged(m_runtimePlatform);
    }
}

QString PlatformStatus::shellPackage() const
{
    return m_shellPackage;
}

QStringList PlatformStatus::runtimePlatform() const
{
    return m_runtimePlatform;
}

void PlatformStatus::fileDirtied(const QString &path)
{
    if (path.endsWith("kdeglobals")) {
        findShellPackage(true);
    }
}

void PlatformStatus::registerClient(const QString & dbus)
{
    if (!m_clients.contains(dbus))
        m_clients << dbus;
}

void PlatformStatus::clientChangeStarted(const QString & dbus, int changeId, int maximumTime)
{
    Q_UNUSED(dbus);
    Q_UNUSED(maximumTime);
    Q_UNUSED(changeId);
}

void PlatformStatus::clientChangeFinished(const QString & dbus, int changeId)
{
    Q_UNUSED(dbus);
    Q_UNUSED(changeId);
}

void PlatformStatus::startPlatformChange(const QString & dbus, const QString & platform)
{
    Q_UNUSED(dbus);
    Q_UNUSED(platform);
}

#include "platformstatus.moc"
