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

class KActionCollection;

class ContainmentInterface;
class ConfigPropertyMap;
class QmlObject;
class QSignalMapper;

class WallpaperInterface : public QQuickItem
{
    Q_OBJECT

    //Q_PROPERTY(QString plugin READ plugin WRITE setPlugin NOTIFY pluginChanged)
    Q_PROPERTY(ConfigPropertyMap *configuration READ configuration NOTIFY configurationChanged)

public:
    WallpaperInterface(ContainmentInterface *parent = 0);
    ~WallpaperInterface();

    Plasma::Package package() const;

    ConfigPropertyMap *configuration() const;

    Plasma::ConfigLoader *configScheme();

    QList<QAction*> contextualActions() const;

    Q_INVOKABLE void setAction(const QString &name, const QString &text,
                               const QString &icon = QString(), const QString &shortcut = QString());

    Q_INVOKABLE void removeAction(const QString &name);

    Q_INVOKABLE QAction *action(QString name) const;

Q_SIGNALS:
    void packageChanged();
    void configurationChanged();

private Q_SLOTS:
    void syncWallpaperPackage();
    void executeAction(const QString &name);

private:
    QString m_wallpaperPlugin;
    ContainmentInterface *m_containmentInterface;
    QmlObject *m_qmlObject;
    Plasma::Package m_pkg;
    ConfigPropertyMap *m_configuration;
    Plasma::ConfigLoader *m_configLoader;
    KActionCollection *m_actions;
    QSignalMapper *m_actionSignals;
};

#endif
