/*
 *   Copyright 2013 Marco Martin <mart@kde.org>
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

#ifndef WALLPAPERINTERFACE_H
#define WALLPAPERINTERFACE_H

#include <QQuickItem>

#include <Plasma/Package>

namespace Plasma {
    class ConfigLoader;
}

class ContainmentInterface;
class ConfigPropertyMap;
class QmlObject;

class WallpaperInterface : public QQuickItem
{
    Q_OBJECT

    //Q_PROPERTY(QString plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(QObject* configuration READ configuration CONSTANT)

public:
    WallpaperInterface(ContainmentInterface *parent = 0);
    ~WallpaperInterface();

    Plasma::Package package() const;

    QObject* configuration() const;

    Plasma::ConfigLoader *configScheme();

Q_SIGNALS:
    void packageChanged();

private Q_SLOTS:
    void syncWallpaperPackage();

private:
    ContainmentInterface *m_containmentInterface;
    QmlObject *m_qmlObject;
    Plasma::Package m_pkg;
    ConfigPropertyMap *m_configuration;
    Plasma::ConfigLoader *m_configLoader;
};

#endif
