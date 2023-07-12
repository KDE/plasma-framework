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
class KConfigPropertyMap;

class ContainmentInterface;

namespace Plasma
{
class Containment;
}

namespace PlasmaQuick
{
class SharedQmlEngine;
}

/**
 * @brief This class is exposed to wallpapers as the WallpaperItem root qml item
 *
 * <b>Import Statement</b>
 * @code import org.kde.plasma.plasmoid @endcode
 * @version 2.0
 */
class WallpaperInterface : public QQuickItem
{
    Q_OBJECT

    Q_PROPERTY(QString pluginName READ pluginName CONSTANT)
    Q_PROPERTY(KConfigPropertyMap *configuration READ configuration CONSTANT)
    /**
     * Actions to be added in the desktop context menu. To instantiate QActions in a declarative way,
     * PlasmaCore.Action {} can be used
     */
    Q_PROPERTY(QQmlListProperty<QAction> contextualActions READ qmlContextualActions NOTIFY contextualActionsChanged)
    Q_PROPERTY(bool loading MEMBER m_loading NOTIFY isLoadingChanged)

public:
    explicit WallpaperInterface(QQuickItem *parent = nullptr);
    ~WallpaperInterface() override;

    void classBegin() override;
    void componentComplete() override;

    /**
     * Returns a list of all known wallpapers that can accept the given mimetype
     * @param mimetype the mimetype to search for
     * @param formFactor the format of the wallpaper being search for (e.g. desktop)
     * @return list of wallpapers
     */
    static QList<KPluginMetaData> listWallpaperMetadataForMimetype(const QString &mimetype, const QString &formFactor = QString());

    /**
     * Instantiate the WallpaperInterface for a given containment, using the proper plugin
     */
    static WallpaperInterface *loadWallpaper(ContainmentInterface *ContainmentInterface);

    KPackage::Package kPackage() const;

    QString pluginName() const;

    KConfigPropertyMap *configuration() const;

    KConfigLoader *configScheme();

    void requestOpenUrl(const QUrl &url);

    QList<QAction *> contextualActions() const;

    QML_LIST_PROPERTY_ASSIGN_BEHAVIOR_REPLACE
    QQmlListProperty<QAction> qmlContextualActions();

    bool supportsMimetype(const QString &mimetype) const;

    bool isLoading() const;

Q_SIGNALS:
    void isLoadingChanged();
    void openUrlRequested(const QUrl &url);
    void contextualActionsChanged(const QList<QAction *> &actions);
    void repaintNeeded(const QColor &accentColor = Qt::transparent);

private:
    static void contextualActions_append(QQmlListProperty<QAction> *prop, QAction *action);
    static qsizetype contextualActions_count(QQmlListProperty<QAction> *prop);
    static QAction *contextualActions_at(QQmlListProperty<QAction> *prop, qsizetype idx);
    static void contextualActions_clear(QQmlListProperty<QAction> *prop);
    static void contextualActions_replace(QQmlListProperty<QAction> *prop, qsizetype idx, QAction *action);
    static void contextualActions_removeLast(QQmlListProperty<QAction> *prop);

    QString m_wallpaperPlugin;
    Plasma::Containment *m_containment = nullptr;
    PlasmaQuick::SharedQmlEngine *m_qmlObject = nullptr;
    KPackage::Package m_pkg;
    KConfigPropertyMap *m_configuration = nullptr;
    KConfigLoader *m_configLoader = nullptr;
    QList<QAction *> m_contextualActions;
    bool m_loading = false;
};

#endif
