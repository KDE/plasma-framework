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
class KConfigPropertyMap;

class ContainmentInterface;

namespace PlasmaQuick
{
class SharedQmlEngine;
}

/**
 * @brief This class is exposed to wallpapers as the attached property "wallpaper"
 *
 * <b>Import Statement</b>
 * @code import org.kde.plasma.plasmoid @endcode
 * @version 2.0
 */
class WallpaperInterface : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString pluginName READ pluginName NOTIFY packageChanged)
    Q_PROPERTY(KConfigPropertyMap *configuration READ configuration NOTIFY configurationChanged)
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

    KConfigPropertyMap *configuration() const;

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
    void repaintNeeded(const QColor &accentColor = Qt::transparent);

private Q_SLOTS:
    void syncWallpaperPackage();
    void executeAction(const QString &name);
    void loadFinished();

private:
    QString m_wallpaperPlugin;
    ContainmentInterface *m_containmentInterface;
    PlasmaQuick::SharedQmlEngine *m_qmlObject;
    KPackage::Package m_pkg;
    KConfigPropertyMap *m_configuration;
    KConfigLoader *m_configLoader;
    QHash<QString, QAction *> m_actions;
    bool m_loading = false;

    static QHash<QObject *, WallpaperInterface *> s_rootObjects;
};

QML_DECLARE_TYPEINFO(WallpaperInterface, QML_HAS_ATTACHED_PROPERTIES)

#endif
