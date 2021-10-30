/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef WALLPAPERINTERFACE_H
#define WALLPAPERINTERFACE_H

#include <QQmlEngine>
#include <QQuickItem>

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
    Q_PROPERTY(bool loading MEMBER m_loading NOTIFY isLoadingChanged)

public:
    explicit WallpaperInterface(ContainmentInterface *parent = nullptr);
    ~WallpaperInterface() override;

    /**
     * Returns a list of all known wallpapers that can accept the given mimetype
     * @param mimetype the mimetype to search for
     * @param formFactor the format of the wallpaper being search for (e.g. desktop)
     * @return list of wallpapers
     */
    static QList<KPluginMetaData> listWallpaperMetadataForMimetype(const QString &mimetype, const QString &formFactor = QString());

    KPackage::Package kPackage() const;

    QString pluginName() const;

    KDeclarative::ConfigPropertyMap *configuration() const;

    KConfigLoader *configScheme();

    QList<QAction *> contextualActions() const;

    bool supportsMimetype(const QString &mimetype) const;

    void setUrl(const QUrl &urls);

    Q_INVOKABLE void setAction(const QString &name, const QString &text, const QString &icon = QString(), const QString &shortcut = QString());

    Q_INVOKABLE void removeAction(const QString &name);

    Q_INVOKABLE QAction *action(QString name) const;

    static WallpaperInterface *qmlAttachedProperties(QObject *object);

    bool isLoading() const;

Q_SIGNALS:
    void packageChanged();
    void configurationChanged();
    void isLoadingChanged();

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
    bool m_loading = false;

    static QHash<QObject *, WallpaperInterface *> s_rootObjects;
};

QML_DECLARE_TYPEINFO(WallpaperInterface, QML_HAS_ATTACHED_PROPERTIES)

#endif
