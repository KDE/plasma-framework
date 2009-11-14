/*
 *   Copyright 2008 Marco Martin <notmart@gmail.com>
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

#ifndef PLASMA_TABBAR_H
#define PLASMA_TABBAR_H

#include <QtGui/QGraphicsWidget>

#include <ktabbar.h>

#include <plasma/plasma_export.h>

class QString;
class QIcon;

namespace Plasma
{

class TabBarPrivate;

/**
 * @class TabBar plasma/widgets/tabbar.h <Plasma/Widgets/TabBar>
 *
 * @short A tab bar widget, to be used for tabbed interfaces.
 *
 * Provides a Tab bar for use in a tabbed interface where each page is a QGraphicsLayoutItem.
 * Only one of them is displayed at a given time. It is possible to add and remove tabs
 * or modify their text label or their icon.
 */
class PLASMA_EXPORT TabBar : public QGraphicsWidget
{
    Q_OBJECT

    Q_PROPERTY(KTabBar *nativeWidget READ nativeWidget)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex)
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(QString styleSheet READ styleSheet WRITE setStyleSheet)
    Q_PROPERTY(bool tabBarShown READ isTabBarShown WRITE setTabBarShown)

public:
    /**
     * Constructs a new TabBar
     *
     * @arg parent the parent of this widget
     */
    explicit TabBar(QGraphicsWidget *parent = 0);
    ~TabBar();

    /**
     * Adds a new tab in the desired position
     *
     * @arg index the position where to insert the new tab,
     *            if index <=0 will be the first position,
     *            if index >= count() will be the last
     * @arg icon  the icon for this tab
     * @arg label the text label of the tab
     * @arg content the page content that will be shown by this tab
     * @return the index of the inserted tab
     */
    Q_INVOKABLE int insertTab(int index, const QIcon &icon, const QString &label,
                              QGraphicsLayoutItem *content = 0);

    /**
     * Adds a new tab in the desired position
     * This is an overloaded member provided for convenience
     * equivalent to insertTab(index, QIcon(), label);
     *
     * @arg index the position where to insert the new tab,
     *            if index <=0 will be the first position,
     *            if index >= count() will be the last
     * @arg label the text label of the tab
     * @arg content the page content that will be shown by this tab
     * @return the index of the inserted tab
     */
    Q_INVOKABLE int insertTab(int index, const QString &label, QGraphicsLayoutItem *content = 0);

    /**
     * Adds a new tab in the last position
     *
     * @arg icon  the icon for this tab
     * @arg label the text label of the tab
     * @arg content the page content that will be shown by this tab
     * @return the index of the inserted tab
     */
    Q_INVOKABLE int addTab(const QIcon &icon, const QString &label, QGraphicsLayoutItem *content = 0);

    /**
     * Adds a new tab in the last position
     * This is an overloaded member provided for convenience
     * equivalent to addTab(QIcon(), label, page)
     *
     * @arg label the text label of the tab
     * @arg content the page content that will be shown by this tab
     * @return the index of the inserted tab
     */
    Q_INVOKABLE int addTab(const QString &label, QGraphicsLayoutItem *content = 0);

    /**
     * Removes a tab, contents are deleted
     *
     * @arg index the index of the tab to remove
     */
    Q_INVOKABLE void removeTab(int index);

    /**
     * Removes a tab, the page is reparented to 0 and is returned
     *
     * @arg index the index of the tab to remove
     * @since 4.4
     */
    Q_INVOKABLE QGraphicsLayoutItem *takeTab(int index);

    /**
     * Returns the contents of a page
     *
     * @arg index the index of the tab to retrieve
     * @since 4.4
     */
    Q_INVOKABLE QGraphicsLayoutItem *tabAt(int index);

    /**
     * @return the index of the tab currently active
     */
    int currentIndex() const;

    /**
     * @return the number of tabs in this tabbar
     */
    int count() const;

    /**
     * Sets the text label of the given tab
     *
     * @arg index the index of the tab to modify
     * @arg label the new text label of the given tab
     */
    Q_INVOKABLE void setTabText(int index, const QString &label);

    /**
     * @return the text label of the given tab
     *
     * @arg index the index of the tab we want to know its label
     */
    Q_INVOKABLE QString tabText(int index) const;

    /**
     * Sets an icon for a given tab
     *
     * @arg index the index of the tab to modify
     * @arg icon the new icon for the given tab
     */
    Q_INVOKABLE void setTabIcon(int index, const QIcon &icon);

    /**
     * @return the current icon for a given tab
     *
     * @arg index the index of the tab we want to know its icon
     */
    Q_INVOKABLE QIcon tabIcon(int index) const;

    /**
     * shows or hides the tabbar, used if you just want to display the
     * pages, when the tabbar doesn't have content pages at all this
     * function has no effect
     *
     * @arg show true if we want to show the tabbar
     * @since 4.3
     */
     void setTabBarShown(bool show);

     /**
      * @return true if the tabbar is shown
      * @since 4.3
      */
     bool isTabBarShown() const;

    /**
     * Sets the stylesheet used to control the visual display of this TabBar
     *
     * @arg stylesheet a CSS string
     */
    void setStyleSheet(const QString &stylesheet);

    /**
     * @return the stylesheet currently used with this widget
     */
    QString styleSheet() const;

    /**
     * @return the native widget wrapped by this TabBar
     */
    KTabBar *nativeWidget() const;

public Q_SLOTS:
    /**
     * Activate a given tab
     *
     * @arg index the index of the tab to activate
     */
    void setCurrentIndex(int index);

Q_SIGNALS:
    /**
     * Emitted when the active tab changes
     *
     * @arg index the newly activated tab
     */
    void currentChanged(int index);

protected:
    void wheelEvent(QGraphicsSceneWheelEvent *event);
    void resizeEvent(QGraphicsSceneResizeEvent * event);
    void changeEvent(QEvent *event);

private:
    TabBarPrivate * const d;

    friend class TabBarPrivate;

    Q_PRIVATE_SLOT(d, void slidingCompleted(QGraphicsItem *item))
    Q_PRIVATE_SLOT(d, void shapeChanged(const QTabBar::Shape shape))
    Q_PRIVATE_SLOT(d, void setPalette())
};

} // namespace Plasma

#endif // multiple inclusion guard
