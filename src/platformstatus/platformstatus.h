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

#ifndef PLATFORMSTATUS_H
#define PLATFORMSTATUS_H

#include <KDEDModule>

#include <QStringList>

class PlatformStatus : public KDEDModule
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.PlatformStatus")
    Q_PROPERTY(QString shellPackage READ shellPackage NOTIFY shellPackageChanged)
    Q_PROPERTY(QStringList runtimePlatform READ runtimePlatform NOTIFY runtimePlatformChanged)

public:
    PlatformStatus(QObject *parent, const QVariantList &);

    ~PlatformStatus();

public Q_SLOTS:
    /**
     * Registers a client interested in the
     * platform changes.
     * @arg dbus dbus object of the client
     */
    void registerClient(const QString & dbus);

    /**
     * Notifies the service that the client
     * wants to react to the platform changes.
     * @arg dbus dbus object of the client
     * @arg changeId id of the change the client is interested in
     * @arg maximumTime maximum time it should take
     * the client to adapt.
     */
    void clientChangeStarted(const QString & dbus, int changeId, int maximumTime);

    /**
     * Notifies the service that the client
     * ended adapting to the platform change.
     * @arg dbus dbus object of the client
     * @arg changeId id of the change the client is interested in
     */
    void clientChangeFinished(const QString & dbus, int changeId);

    /**
     * Starting the platform change.
     * This method should mainly be invoked by plasma shell.
     * @arg dbus the requesting client
     * @arg platform new platform
     */
    void startPlatformChange(const QString & dbus, const QString & platform);

    /**
     * Invoked when the platform change has finished
     */
    void changeProcessFinished(int id);


Q_SIGNALS:
    /**
     * The platform is about to change.
     * The clients need to respond to this signal
     * and notify the service if it wants to react
     * to the platform change.
     * This will allow the service to tell whether
     * all platform changes have finished.
     * @see clientChangeStarted
     * @arg platform new platform that is being loaded
     * @arg changeId identifier of a particular change
     */
    void platformAboutToChange(int changeId, const QString & platform);

    /**
     * Signals when the change has finished
     */
    void platformChanged(const QString & platform, int changeId);

public Q_SLOTS:
    QString shellPackage() const;
    QStringList runtimePlatform() const;

Q_SIGNALS:
    void shellPackageChanged(const QString &package);
    void runtimePlatformChanged(const QStringList &runtimePlatform);

private:
    void findShellPackage(bool sendSignal);

private Q_SLOTS:
    void fileDirtied(const QString &path);

private:
    class Private;
    const QScopedPointer<Private> d;
};

#endif

