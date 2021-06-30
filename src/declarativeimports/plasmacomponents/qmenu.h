/*
    SPDX-FileCopyrightText: 2011 Viranch Mehta <viranch.mehta@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef QMENU_PROXY_H
#define QMENU_PROXY_H

#include "enums.h"
#include "plasma.h"
#include "qmenuitem.h"
#include <QMenu>
#include <QObject>
#include <QPointer>
#include <QQmlListProperty>
#include <QWindow>

class QDeclarativeItem;

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
 * import org.kde.plasma.components 2.0 as PlasmaComponents
 *
 * [...]
 * PlasmaComponents.Menu {
 *     id: menu
 *     ...
 *     PlasmaComponents.MenuItem {
 *         text: "Delete"
 *         onClicked: {
 *             myListItem.remove();
 *         }
 *     }
 * }
 * PlasmaComponents.Button {
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
    Q_PROPERTY(DialogStatus::Status status READ status NOTIFY statusChanged)

    /**
     * The default placement for the menu.
     */
    Q_PROPERTY(Plasma::Types::PopupPlacement placement READ placement WRITE setPlacement NOTIFY placementChanged)

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
    explicit QMenuProxy(QObject *parent = nullptr);
    ~QMenuProxy() override;

    QQmlListProperty<QMenuItem> content();
    int actionCount() const;
    QMenuItem *action(int) const;
    DialogStatus::Status status() const;

    QObject *visualParent() const;
    void setVisualParent(QObject *parent);

    QWindow *transientParent();
    void setTransientParent(QWindow *parent);

    Plasma::Types::PopupPlacement placement() const;
    void setPlacement(Plasma::Types::PopupPlacement placement);

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

    QList<QMenuItem *> m_items;
    QMenu *m_menu;
    DialogStatus::Status m_status;
    QPointer<QObject> m_visualParent;
    Plasma::Types::PopupPlacement m_placement;
};

#endif // QMENU_PROXY_H
