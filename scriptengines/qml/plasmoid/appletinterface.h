/*
 *   Copyright 2008 Chani Armitage <chani@kde.org>
 *   Copyright 2008, 2009 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Marco Martin <mart@kde.org>
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

#ifndef APPLETINTERFACE_H
#define APPLETINTERFACE_H

#include <QAbstractAnimation>
#include <QQuickItem>
#include <QScriptValue>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Theme>

#include "declarativeappletscript.h"

class QAction;
class QmlAppletScript;
class QSignalMapper;
class QSizeF;


namespace Plasma
{
    class ConfigLoader;
} // namespace Plasa

class AppletInterface : public QQuickItem
{
    Q_OBJECT
    Q_ENUMS(FormFactor)
    Q_ENUMS(Location)
    Q_ENUMS(BackgroundHints)
    Q_ENUMS(AnimationDirection)
    Q_ENUMS(IntervalAlignment)
    Q_ENUMS(ThemeColors)
    Q_ENUMS(ItemStatus)

    Q_PROPERTY(FormFactor formFactor READ formFactor NOTIFY formFactorChanged)
    Q_PROPERTY(Location location READ location NOTIFY locationChanged)
    Q_PROPERTY(QString currentActivity READ currentActivity NOTIFY contextChanged)
    Q_PROPERTY(QString activeConfig WRITE setActiveConfig READ activeConfig)
    Q_PROPERTY(bool busy WRITE setBusy READ isBusy NOTIFY busyChanged)
    Q_PROPERTY(BackgroundHints backgroundHints WRITE setBackgroundHints READ backgroundHints NOTIFY backgroundHintsChanged)
    Q_PROPERTY(bool immutable READ immutable NOTIFY immutableChanged)
    Q_PROPERTY(bool userConfiguring READ userConfiguring) // @since 4.5
    Q_PROPERTY(int apiVersion READ apiVersion CONSTANT)
    Q_PROPERTY(ItemStatus status READ status WRITE setStatus NOTIFY statusChanged)
    Q_PROPERTY(QString associatedApplication WRITE setAssociatedApplication READ associatedApplication)

public:
    AppletInterface(DeclarativeAppletScript *script, QQuickItem *parent = 0);
    ~AppletInterface();

//------------------------------------------------------------------
//enums copy&pasted from plasma.h because qtscript is evil

enum FormFactor {
    Planar = 0,  /**< The applet lives in a plane and has two
                    degrees of freedom to grow. Optimize for
                    desktop, laptop or tablet usage: a high
                    resolution screen 1-3 feet distant from the
                    viewer. */
    MediaCenter, /**< As with Planar, the applet lives in a plane
                    but the interface should be optimized for
                    medium-to-high resolution screens that are
                    5-15 feet distant from the viewer. Sometimes
                    referred to as a "ten foot interface".*/
    Horizontal,  /**< The applet is constrained vertically, but
                    can expand horizontally. */
    Vertical,     /**< The applet is constrained horizontally, but
                    can expand vertically. */
    Application /**< The Applet lives in a plane and should be optimized to look as a full application,
                     for the desktop or the particular device. */
};

enum Location {
    Floating = 0, /**< Free floating. Neither geometry or z-ordering
                     is described precisely by this value. */
    Desktop,      /**< On the planar desktop layer, extending across
                     the full screen from edge to edge */
    FullScreen,   /**< Full screen */
    TopEdge,      /**< Along the top of the screen*/
    BottomEdge,   /**< Along the bottom of the screen*/
    LeftEdge,     /**< Along the left side of the screen */
    RightEdge     /**< Along the right side of the screen */
};

enum ItemStatus {
    UnknownStatus = 0, /**< The status is unknown **/
    PassiveStatus = 1, /**< The Item is passive **/
    ActiveStatus = 2, /**< The Item is active **/
    NeedsAttentionStatus = 3, /**< The Item needs attention **/
    AcceptingInputStatus = 4 /**< The Item is accepting input **/
};

enum BackgroundHints {
    NoBackground = Plasma::NoBackground,
    StandardBackground = Plasma::StandardBackground,
    TranslucentBackground = Plasma::TranslucentBackground,
    DefaultBackground = Plasma::DefaultBackground
};

enum ThemeColors {
    TextColor = Plasma::Theme::TextColor,
    HighlightColor = Plasma::Theme::HighlightColor,
    BackgroundColor = Plasma::Theme::BackgroundColor,
    ButtonTextColor = Plasma::Theme::ButtonTextColor,
    ButtonBackgroundColor = Plasma::Theme::ButtonBackgroundColor,
    LinkColor = Plasma::Theme::LinkColor,
    VisitedLinkColor = Plasma::Theme::VisitedLinkColor
};

enum AnimationDirection {
    AnimationForward = QAbstractAnimation::Forward,
    AnimationBackward = QAbstractAnimation::Backward
};

enum IntervalAlignment {
    NoAlignment = 0,
    AlignToMinute,
    AlignToHour
};

//-------------------------------------------------------------------

    Q_INVOKABLE void setConfigurationRequired(bool needsConfiguring, const QString &reason = QString());

    Q_INVOKABLE void setActionSeparator(const QString &name);
    Q_INVOKABLE void setAction(const QString &name, const QString &text,
                               const QString &icon = QString(), const QString &shortcut = QString());

    Q_INVOKABLE void removeAction(const QString &name);

    Q_INVOKABLE QAction *action(QString name) const;

    Q_INVOKABLE QVariant readConfig(const QString &entry) const;

    Q_INVOKABLE void writeConfig(const QString &entry, const QVariant &value);

    Q_INVOKABLE QString file(const QString &fileType);
    Q_INVOKABLE QString file(const QString &fileType, const QString &filePath);

    Q_INVOKABLE bool include(const QString &script);

    Q_INVOKABLE void debug(const QString &msg);

    QList<QAction*> contextualActions() const;

    inline Plasma::Applet *applet() const { return m_appletScriptEngine->applet(); }

//    Q_INVOKABLE QString downloadPath(const QString &file);
    Q_INVOKABLE QStringList downloadedFiles() const;


//PROPERTY ACCESSORS
    FormFactor formFactor() const;

    Location location() const;

    QString currentActivity() const;

    bool isBusy() const;
    void setBusy(bool busy);

    BackgroundHints backgroundHints() const;
    void setBackgroundHints(BackgroundHints hint);

    void setAssociatedApplication(const QString &string);
    QString associatedApplication() const;

    void setStatus(const ItemStatus &status);
    ItemStatus status() const;

    QString activeConfig() const;
    void setActiveConfig(const QString &name);

    bool immutable() const;
    bool userConfiguring() const;
    int apiVersion() const;

Q_SIGNALS:
    void releaseVisualFocus();
    void configNeedsSaving();

    void formFactorChanged();
    void locationChanged();
    void contextChanged();
    void immutableChanged();
    void statusChanged();
    void backgroundHintsChanged();
    void busyChanged();

protected:
    DeclarativeAppletScript *m_appletScriptEngine;

private:
    QStringList m_actions;
    QSignalMapper *m_actionSignals;
    QString m_currentConfig;
    QMap<QString, Plasma::ConfigLoader*> m_configs;

//UI-specific properties
    Plasma::BackgroundHints m_backgroundHints;
    bool m_busy : 1;
};


class ContainmentInterface : public AppletInterface
{
    Q_OBJECT
    Q_PROPERTY(QVariantList applets READ applets)
    Q_PROPERTY(bool drawWallpaper READ drawWallpaper WRITE setDrawWallpaper)
    Q_PROPERTY(Type containmentType READ containmentType WRITE setContainmentType)
    Q_PROPERTY(int screen READ screen NOTIFY screenChanged)
    Q_PROPERTY(QString activityId READ activityId NOTIFY activityIdChanged)
    Q_ENUMS(Type)

public:
    enum Type {
        NoContainmentType = -1,  /**< @internal */
        DesktopContainment = 0,  /**< A desktop containment */
        PanelContainment,        /**< A desktop panel */
        CustomContainment = 127, /**< A containment that is neither a desktop nor a panel
                                    but something application specific */
        CustomPanelContainment = 128 /**< A customized desktop panel */
    };
    ContainmentInterface(DeclarativeAppletScript *parent);

    inline Plasma::Containment *containment() const { return static_cast<Plasma::Containment *>(m_appletScriptEngine->applet()); }

    QVariantList applets();

    void setDrawWallpaper(bool drawWallpaper);
    bool drawWallpaper();
    Type containmentType() const;
    void setContainmentType(Type type);
    int screen() const;

    QString activityId() const;

    Q_INVOKABLE QRectF screenGeometry(int id) const;
    Q_INVOKABLE QVariantList availableScreenRegion(int id) const;

Q_SIGNALS:
    void appletAdded(QObject *applet, const QPointF &pos);
    void appletRemoved(QObject *applet);
    void screenChanged();
    void activityIdChanged();
    void availableScreenRegionChanged();

protected Q_SLOTS:
    void appletAddedForward(Plasma::Applet *applet, const QPointF &pos);
    void appletRemovedForward(Plasma::Applet *applet);
    void loadWallpaper();

private:
    bool m_movableApplets;
};

#endif
