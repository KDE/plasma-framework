/*
 * Copyright 2008 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef PLASMA_EXTENDERITEM_H
#define PLASMA_EXTENDERITEM_H

#include <QtGui/QGraphicsWidget>

#include <KDE/KConfigGroup>
#include <KDE/KIcon>

#include "plasma/plasma_export.h"

namespace Plasma
{

class Applet;
class Extender;
class ExtenderItemPrivate;

/**
 * @class ExtenderItem plasma/extenderitem.h <Plasma/ExtenderItem>
 *
 * @short Provides detachable items for an Extender
 *
 * This class wraps around a QGraphicsWidget and provides drag&drop handling, a draghandle,
 * title and ability to display qactions as a row of icon, ability to expand, collapse, return
 * to source and tracks configuration associated with this item for you.
 */
class PLASMA_EXPORT ExtenderItem : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(QGraphicsItem * widget READ widget WRITE setWidget)
    Q_PROPERTY(QString title READ title WRITE setTitle)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(QIcon icon READ icon WRITE setIcon)
    Q_PROPERTY(Extender * extender READ extender WRITE setExtender)
    Q_PROPERTY(bool collapsed READ isCollapsed WRITE setCollapsed)
    Q_PROPERTY(bool detached READ isDetached)
    Q_PROPERTY(uint autoExpireDelay WRITE setAutoExpireDelay)

    public:
        /**
         * The constructor takes care of adding this item to an extender.
         * @param hostExtender The extender where the extender item belongs to.
         * TODO: extenderItemId might not be necesarry in the constructor if I rewrite some
         * stuff.
         * @param extenderItemId the id of the extender item. Use the default 0 to assign a new,
         * unique id to this extender item.
         */
        ExtenderItem(Extender *hostExtender, uint extenderItemId = 0);

        ~ExtenderItem();

        /**
         * fetch the configuration of this widget.
         * @return the configuration of this widget.
         */
        KConfigGroup config() const;

        /**
         * @param widget The widget that should be wrapped into the extender item.
         */
        void setWidget(QGraphicsItem *widget);

        /**
         * @return The widget that is wrapped into the extender item.
         */
        QGraphicsItem *widget() const;

        /**
         * @param title the title that will be shown in the extender item's dragger. Default is
         * no title.
         */
        void setTitle(const QString &title);

        /**
         * @return the title shown in the extender item's dragger.
         */
        QString title() const;

        /**
         * You can assign names to extender items to look them up through the item() function.
         * Make sure you only use unique names.
         * @param name the name of the item. Defaults to an empty string.
         */
        void setName(const QString &name);

        /**
         * @return the name of the item.
         */
        QString name() const;

        /**
         * @param icon the icon name to display in the extender item's
         * drag handle. Defaults to the source applet's icon.
         */
        void setIcon(const QString &icon);

        /**
         * @param icon the icon to display in the extender item's drag handle. Defaults to the
         * source applet's icon.
         */
        void setIcon(const QIcon &icon);

        /**
         * @return the icon being displayed in the extender item's drag handle.
         */
        QIcon icon() const;

        /**
         * @param extender the extender this item belongs to.
         * @param pos the position in the extender this item should be added. Defaults to 'just
         * append'.
         */
        void setExtender(Extender *extender, const QPointF &pos = QPointF(-1, -1));

        /**
         * @return the extender this items belongs to.
         */
        Extender *extender() const;

        /**
         * @param time (in ms) before this extender item destroys itself unless it is detached,
         * in which case this extender stays around. 0 means forever and is the default.
         */
        void setAutoExpireDelay(uint time);

        /**
         * @return whether or not this extender item has an auto expire delay.
         */
        bool autoExpireDelay() const;

        /**
         * @return whether or not this item is detached from it's original source.
         */
        bool isDetached() const;

        /**
         * @return whether or not the extender item  is collapsed.
         */
        bool isCollapsed() const;

        /**
         * @param name the name to store the action under in our collection.
         * @param action the action to add. Actions will be displayed as an icon in the drag
         * handle.
         */
        void addAction(const QString &name, QAction *action);

        /**
         * @return the QAction with the given name from our collection. By default the action
         * collection contains a "movebacktosource" action which will be only shown when the
         * item is detached.
         */
        QAction *action(const QString &name) const;

        /**
         * @return the id of the applet this item is created by.
         */
        uint sourceAppletId() const;

    public Q_SLOTS:
        /**
         * Destroys the extender item. As opposed to calling delete on this class, destroy also
         * removes the config group associated with this item.
         */
        void destroy();

        /**
         * Collapse or expand the extender item. Defaults to false.
         */
        void setCollapsed(bool collapsed);

        /**
         * Returns the extender item to it's source applet.
         */
        void moveBackToSource();

    protected:
        void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

        void resizeEvent(QGraphicsSceneResizeEvent *event);

        void mousePressEvent(QGraphicsSceneMouseEvent *event);
        void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        void hoverMoveEvent(QGraphicsSceneHoverEvent *event);
        void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

    private:
        Q_PRIVATE_SLOT(d, void toggleCollapse())
        Q_PRIVATE_SLOT(d, void updateToolBox())
        Q_PRIVATE_SLOT(d, void themeChanged())

        ExtenderItemPrivate * const d;

        friend class Extender;
        friend class ExtenderPrivate;
};
} // namespace Plasma
#endif // PLASMA_EXTENDERITEM_H
