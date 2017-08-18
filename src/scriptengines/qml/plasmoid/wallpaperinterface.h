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
#include <QQmlEngine>

#include <KPackage/Package>

class KConfigLoader;
class KActionCollection;

class ContainmentInterface;

namespace KDeclarative
{
class ConfigPropertyMap;
class QmlObject;
}

/**
 * @class WallpaperInterface
 *
 * @brief This class is exposed to wallpapers as the attached property "wallpaper"
 *
 *
 */
class WallpaperInterface : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString pluginName READ pluginName NOTIFY packageChanged)
    Q_PROPERTY(KDeclarative::ConfigPropertyMap *configuration READ configuration NOTIFY configurationChanged)

public:
    WallpaperInterface(ContainmentInterface *parent = 0);
    ~WallpaperInterface();

    /**
     * Returns a list of all known wallpapers that can accept the given mimetype
     * @param mimetype the mimetype to search for
     * @param formFactor the format of the wallpaper being search for (e.g. desktop)
     * @return list of wallpapers
     */
    static QList<KPluginMetaData> listWallpaperMetadataForMimetype(const QString &mimetype,
            const QString &formFactor = QString());

    KPackage::Package kPackage() const;

    QString pluginName() const;

    KDeclarative::ConfigPropertyMap *configuration() const;

    KConfigLoader *configScheme();

    QList<QAction *> contextualActions() const;

    bool supportsMimetype(const QString &mimetype) const;

    void setUrl(const QUrl &urls);

    Q_INVOKABLE void setAction(const QString &name, const QString &text,
                               const QString &icon = QString(), const QString &shortcut = QString());

    Q_INVOKABLE void removeAction(const QString &name);

    Q_INVOKABLE QAction *action(QString name) const;

    static WallpaperInterface *qmlAttachedProperties(QObject *object);

Q_SIGNALS:
    void packageChanged();
    void configurationChanged();

private Q_SLOTS:
    void syncWallpaperPackage();
    void executeAction(const QString &name);
    void loadFinished();

private:
    QString m_wallpaperPlugin;
    ContainmentInterface *m_containmentInterface;
    KDeclarative::QmlObject *m_qmlObject;
    KPackage::Package m_pkg;
    KDeclarative::ConfigPropertyMap *m_configuration;
    KConfigLoader *m_configLoader;
    KActionCollection *m_actions;

    static QHash<QObject *, WallpaperInterface *> s_rootObjects;
};

QML_DECLARE_TYPEINFO(WallpaperInterface, QML_HAS_ATTACHED_PROPERTIES)

#endif
