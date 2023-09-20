/*
    SPDX-FileCopyrightText: 2011 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMENU_PROXY_H
#define QMENU_PROXY_H

#include "plasma.h"
#include "qmenuitem.h"
#include <QMenu>
#include <QObject>
#include <QPointer>
#include <QQmlListProperty>
#include <QWindow>

class QDeclarativeItem;
class QQuickItem;

/**
 * @class Menu
 *
 * An Item provides a menu for use in context specific situations.
 * You can specify the position for the menu to open by setting its visualParent.
 * MenuItems should be used to draw entries in the menu.
 * The open() function opens up the menu at the given visualParent.
 *
 *
 * Example usage:
 * @code
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 *
 * [...]
 * PlasmaExtras.Menu {
 *     id: menu
 *     ...
 *     PlasmaExtras.MenuItem {
 *         text: "Delete"
 *         onClicked: {
 *             myListItem.remove();
 *         }
 *     }
 * }
 * PlasmaExtras.Button {
 *     id: btn
 *     onClicked: {
 *         menu.visualParent = btn
 *         menu.open()
 *     }
 * }
 * [...]
 * @endcode
 *
 */

class QMenuProxy : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QQmlListProperty<QMenuItem> content READ content CONSTANT)
    Q_CLASSINFO("DefaultProperty", "content")

    /**
     * This is a hint to the window manager that this window is a dialog or pop-up on
     * behalf of the given window.
     */
    Q_PROPERTY(QWindow *transientParent READ transientParent WRITE setTransientParent NOTIFY transientParentChanged)

    /**
     * The visualParent is used to position the menu. it can be an item on the
     * scene, like a button (that will open the menu on clicked) or another menuitem
     * (in this case this will be a submenu).
     */
    Q_PROPERTY(QObject *visualParent READ visualParent WRITE setVisualParent NOTIFY visualParentChanged())
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)

    /**
     * The default placement for the menu.
     *
     * (since 5.103) Will be automatically flipped horizontally in Right-to-left User Interfaces.
     */
    Q_PROPERTY(PopupPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)

    /**
     * A minimum width for the menu.
     */
    Q_PROPERTY(int minimumWidth READ minimumWidth WRITE setMinimumWidth NOTIFY minimumWidthChanged)

    /**
     * A maximum width for the menu.
     *
     * @since 5.31
     */
    Q_PROPERTY(int maximumWidth READ maximumWidth WRITE setMaximumWidth RESET resetMaximumWidth NOTIFY maximumWidthChanged)

public:
    /**
     * The popup position enumeration relatively to his attached widget
     *
     **/
    enum PopupPlacement {
        FloatingPopup = 0, /**< Free floating, non attached popup */
        TopPosedLeftAlignedPopup, /**< Popup positioned on the top, aligned
                                 to the left of the widget */
        TopPosedRightAlignedPopup, /**< Popup positioned on the top, aligned
                                  to the right of the widget */
        LeftPosedTopAlignedPopup, /**< Popup positioned on the left, aligned
                                 to the top of the widget */
        LeftPosedBottomAlignedPopup, /**< Popup positioned on the left, aligned
                                    to the bottom of the widget */
        BottomPosedLeftAlignedPopup, /**< Popup positioned on the bottom, aligned
                                    to the left of the widget */
        BottomPosedRightAlignedPopup, /**< Popup positioned on the bottom, aligned
                                     to the right of the widget */
        RightPosedTopAlignedPopup, /**< Popup positioned on the right, aligned
                                  to the top of the widget */
        RightPosedBottomAlignedPopup, /**< Popup positioned on the right, aligned
                                    to the bottom of the widget */
    };
    Q_ENUM(PopupPlacement)

    enum Status {
        Opening,
        Open,
        Closing,
        Closed,
    };
    Q_ENUM(Status)

    explicit QMenuProxy(QObject *parent = nullptr);
    ~QMenuProxy() override;

    QQmlListProperty<QMenuItem> content();
    int actionCount() const;
    QMenuItem *action(int) const;
    Status status() const;

    QObject *visualParent() const;
    void setVisualParent(QObject *parent);

    QWindow *transientParent();
    void setTransientParent(QWindow *parent);

    PopupPlacement placement() const;
    void setPlacement(PopupPlacement placement);

    int minimumWidth() const;
    void setMinimumWidth(int width);

    int maximumWidth() const;
    void setMaximumWidth(int maximumWidth);
    void resetMaximumWidth();

    /**
     * This opens the menu at position x,y on the given visualParent. By default x and y are set to 0
     */
    Q_INVOKABLE void open(int x = 0, int y = 0);
    /**
     * This opens the menu at the specified placement relative to the visualParent.
     */
    Q_INVOKABLE void openRelative();
    /**
     * This closes the menu
     */
    Q_INVOKABLE void close();
    /**
     * This removes all menuItems inside the menu
     */
    Q_INVOKABLE void clearMenuItems();
    /**
     * This adds a menu item from a String
     */
    Q_INVOKABLE void addMenuItem(const QString &text);
    /**
     * This adds MenuItem 'item' to the menu before MenuItem 'before'.
     * If MenuItem 'before' is 0 or does not exist in the menu, 'item'
     * is appended to the menu instead.
     * If MenuItem 'item' already exists in the menu, it is removed and
     * inserted at the new position.
     */
    Q_INVOKABLE void addMenuItem(QMenuItem *item, QMenuItem *before = nullptr);
    /**
     * This adds a section header with a string used as name for the section
     */
    Q_INVOKABLE void addSection(const QString &text);

    /**
     * This removes MenuItem 'item'
     *
     * @since 5.27
     */
    Q_INVOKABLE void removeMenuItem(QMenuItem *item);

protected:
    bool event(QEvent *event) override;

Q_SIGNALS:
    void statusChanged();
    void visualParentChanged();
    void transientParentChanged();
    void placementChanged();
    void minimumWidthChanged();
    void maximumWidthChanged();
    void triggered(QMenuItem *item);
    void triggeredIndex(int index);

private Q_SLOTS:
    void itemTriggered(QAction *item);

private:
    void rebuildMenu();
    void openInternal(QPoint pos);
    QQuickItem *parentItem() const;
    /*
     * Mirrors popup placement horizontally in Right-To-Left environments.
     *
     * Mirroring behavior can be explicitly overridden by passing a specific
     * direction with layoutDirection parameter, or left at default value of
     * Qt::LayoutDirectionAuto, in which case it will be deduced from shared
     * QGuiApplication instance.
     **/
    PopupPlacement visualPopupPlacement(PopupPlacement placement, Qt::LayoutDirection layoutDirection = Qt::LayoutDirectionAuto);

    QList<QMenuItem *> m_items;
    QMenu *m_menu;
    Status m_status;
    QPointer<QObject> m_visualParent;
    PopupPlacement m_placement;
};

#endif // QMENU_PROXY_H
