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
    QString m_shellPackage;
    QStringList m_runtimePlatform;
};

#endif

