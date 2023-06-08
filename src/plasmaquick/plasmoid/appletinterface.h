/*
    SPDX-FileCopyrightText: 2008 Chani Armitage <chani@kde.org>
    SPDX-FileCopyrightText: 2008, 2009 Aaron Seigo <aseigo@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef APPLETINTERFACE_H
#define APPLETINTERFACE_H

#include <QAction>
#include <QQuickItem>
#include <QQuickView>

#include <KPluginMetaData>

#include <Plasma/Applet>
#include <Plasma/Containment>
#include <Plasma/Theme>

#include <appletquickitem.h>

class QActionGroup;
class QSizeF;
class KConfigPropertyMap;

namespace Plasma
{
class ConfigLoader;
} // namespace Plasma

/**
 * @class AppletInterface
 *
 * @short This class is exposed to applets in QML as the attached property Plasmoid
 *
 * <b>Import Statement</b>
 * @code import org.kde.plasma.plasmoid @endcode
 * @version 2.0
 */
class AppletInterface : public PlasmaQuick::AppletQuickItem
{
    Q_OBJECT

    /**
     * The QML root object defined in the applet main.qml will be direct child of an AppletInterface instance
     */

    /**
     * Main title for the plasmoid tooltip or other means of quick information:
     * it's the same as the title property by default, but it can be personalized
     */
    Q_PROPERTY(QString toolTipMainText READ toolTipMainText WRITE setToolTipMainText NOTIFY toolTipMainTextChanged)

    /**
     * Description for the plasmoid tooltip or other means of quick information:
     * it comes from the pluginifo comment by default, but it can be personalized
     */
    Q_PROPERTY(QString toolTipSubText READ toolTipSubText WRITE setToolTipSubText NOTIFY toolTipSubTextChanged)

    /**
     * how to handle the text format of the tooltip subtext:
     * * Text.AutoText (default)
     * * Text.PlainText
     * * Text.StyledText
     * * Text.RichText
     * Note: in the default implementation the main text is always plain text
     */
    Q_PROPERTY(int toolTipTextFormat READ toolTipTextFormat WRITE setToolTipTextFormat NOTIFY toolTipTextFormatChanged)

    /**
     * This allows to set fully custom QML item as the tooltip.
     * It will ignore all texts set by setToolTipMainText or setToolTipSubText
     *
     * @since 5.19
     */
    Q_PROPERTY(QQuickItem *toolTipItem READ toolTipItem WRITE setToolTipItem NOTIFY toolTipItemChanged)

    // TODO: This was moved up from ContainmentInterface because it is required by the
    // Task Manager applet (for "Show only tasks from this screen") and no Qt API exposes
    // screen numbering. An alternate solution that doesn't extend the applet interface
    // would be preferable if found.
    Q_PROPERTY(int screen READ screen NOTIFY screenChanged)

    /**
     * Provides access to the geometry of the applet is in.
     * Can be useful to figure out what's the absolute position of the applet.
     * TODO: move in containment
     */
    Q_PROPERTY(QRect screenGeometry READ screenGeometry NOTIFY screenGeometryChanged)

    /**
     * Whether the dialog should be hidden when the dialog loses focus.
     *
     * The default value is @c false.
     * TODO KF6: move to Applet? probably not
     **/
    Q_PROPERTY(bool hideOnWindowDeactivate READ hideOnWindowDeactivate WRITE setHideOnWindowDeactivate NOTIFY hideOnWindowDeactivateChanged)

    /**
     * screen area free of panels: the coordinates are relative to the containment,
     * it's independent from the screen position
     * For more precise available geometry use availableScreenRegion()
     */
    Q_PROPERTY(QRect availableScreenRect READ availableScreenRect NOTIFY availableScreenRectChanged)

    /**
     * The available region of this screen, panels excluded. It's a list of rectanglesO: from containment
     */
    Q_PROPERTY(QVariantList availableScreenRegion READ availableScreenRegion NOTIFY availableScreenRegionChanged)

public:
    AppletInterface(QQuickItem *parent = nullptr);
    ~AppletInterface() override;

    // API not intended for the QML part

    void executeAction(QAction *action);

    // QML API-------------------------------------------------------------------

    /**
     * Should be called before retrieving any action
     * to ensure contents are up to date
     * @see contextualActionsAboutToShow
     * @since 5.58
     */
    Q_INVOKABLE void prepareContextualActions();

    QVariantList availableScreenRegion() const;

    QRect availableScreenRect() const;

    // PROPERTY ACCESSORS-------------------------------------------------------------------
    QString pluginName() const;

    QString toolTipMainText() const;
    void setToolTipMainText(const QString &text);

    QString toolTipSubText() const;
    void setToolTipSubText(const QString &text);

    int toolTipTextFormat() const;
    void setToolTipTextFormat(int format);

    QQuickItem *toolTipItem() const;
    void setToolTipItem(QQuickItem *toolTipItem);

    int screen() const;
    QRect screenGeometry() const;

    bool hideOnWindowDeactivate() const;
    void setHideOnWindowDeactivate(bool hide);

Q_SIGNALS:
    /**
     * somebody else, usually the containment sent some data to the applet
     * @param mimetype the mime type of the data such as text/plain
     * @param data either the actual data or an URL representing it
     */
    void externalData(const QString &mimetype, const QVariant &data);

    /**
     * Emitted just before the contextual actions are about to show
     * For instance just before the context menu containing the actions
     * added with setAction() is shown
     */
    void contextualActionsAboutToShow();

    // PROPERTY change notifiers--------------
    void toolTipMainTextChanged();
    void toolTipSubTextChanged();
    void toolTipTextFormatChanged();
    void toolTipItemChanged();
    void screenChanged();
    void screenGeometryChanged();
    void hideOnWindowDeactivateChanged();
    void availableScreenRegionChanged();
    void availableScreenRectChanged();
    void contextualActionsChanged();

protected:
    void init() override;
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void destroyedChanged(bool destroyed);

    QSet<QAction *> m_actions;

    // UI-specific members ------------------

    QString m_toolTipMainText;
    QString m_toolTipSubText;
    int m_toolTipTextFormat;
    QPointer<QQuickItem> m_toolTipItem;
    bool m_hideOnDeactivate : 1;
    int m_oldKeyboardShortcut;

    friend class ContainmentInterface;
    // This is used by ContainmentInterface
    QPointF m_positionBeforeRemoval;
};

#endif
