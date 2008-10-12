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

#ifndef PLASMA_EXTENDER_H
#define PLASMA_EXTENDER_H

#include <QtGui/QGraphicsWidget>

#include "plasma/plasma_export.h"

namespace Plasma
{
class ExtenderPrivate;
class ExtenderItem;
class Applet;

/**
 * @class Extender plasma/extender.h <Plasma/Extender>
 *
 * @short Extends applets to allow detachable parts
 *
 * An Extender is a widget that visually extends the normal contents of an applet with
 * additional dynamic widgets called ExtenderItems. These ExtenderItems can be
 * detached by the user and dropped either on another Extender or on the canvas directly.
 *
 * This widget allows using ExtenderItems in your applet. Extender takes care of the presentation
 * of a collection of ExtenderItems and keeps track of ExtenderItems that originate in it.
 *
 * The default Extender implementation displays extender items in a vertical layout with
 * spacers that appear when dropping an ExtenderItem over it.
 *
 * If you wish to have a different presentation of extender items, you can choose to subclass
 * Extender and reimplement the extenderItem* events and, optionally, the saveState function.
 */
class PLASMA_EXPORT Extender : public QGraphicsWidget
{
    Q_OBJECT
    Q_PROPERTY(QString emptyExtenderMessage READ emptyExtenderMessage WRITE setEmptyExtenderMessage)

    public:
        /**
         * Creates an extender. Note that extender expects applet to have a config(), and needs a
         * scene because of that. So you should only instantiate an extender in init() or later, not
         * in an applet's constructor.
         * @param applet The applet this extender is part of. Null is not allowed here.
         */
        explicit Extender(Applet *applet);

        ~Extender();

        /**
         * @param emptyExtenderMessage The text to be shown whenever the applet's extender is empty.
         * Defaults to i18n'ed "no items".
         */
        void setEmptyExtenderMessage(const QString &message);

        /**
         * @return The text to be shown whenever the applet's layout is empty.
         */
        QString emptyExtenderMessage() const;

        /**
         * @returns a list of all extender items (attached AND detached) where the source applet is
         * this applet.
         */
        QList<ExtenderItem*> items() const;

        /**
         * @returns a list of all attached extender items.
         */
        QList<ExtenderItem*> attachedItems() const;

        /**
         * @returns a list of all detached extender items.
         */
        QList<ExtenderItem*> detachedItems() const;

        /**
         * This function can be used for easily determining if a certain item is already displayed
         * in a extender item somewhere, so your applet doesn't duplicate this item. Say the applet
         * displays 'jobs', from an engine which add's a source for every job. In sourceAdded you
         * could do something like:
         * if (!item(source)) {
         *     //add an extender item monitoring this source.
         * }
         */
        ExtenderItem *item(const QString &name) const;

    protected:
        /**
         * Get's called after an item has been added to this extender. The bookkeeping has already
         * been done when this function get's called. The only thing left to do is put it somewhere
         * appropriate. The default implementation adds the extenderItem to the appropriate place in
         * a QGraphicsLinearLayout.
         * @param item The item that has just been added.
         * @param pos The location the item has been dropped in local coordinates.
         */
        virtual void itemAddedEvent(ExtenderItem *item, const QPointF &pos);

        /**
         * Get's called after an item has been removed from this extender. All bookkeeping has
         * already been done when this function get's called.
         * @param item The item that has just been removed.
         */
        virtual void itemRemovedEvent(ExtenderItem *item);

        /**
         * Get's called when an ExtenderItem that get's dragged enters this extender. Default
         * implementation does nothing.
         */
        virtual void itemHoverEnterEvent(ExtenderItem *item);

        /**
         * Gets called when an ExtenderItem is hovering over this extender. Implement this function
         * to give some visual feedback about what will happen when the mouse button is released at
         * that position. The default implementation shows a spacer at the appropriate location in
         * the layout.
         * @param item The item that's hovering over this extender. Most useful for obtaining the
         * size of the spacer.
         * @param pos The location the item is hovering.
         */
        virtual void itemHoverMoveEvent(ExtenderItem *item, const QPointF &pos);

        /**
         * Get's called when an ExtenderItem that was previously hovering over this extender moves
         * away from this extender. The default implementation removes any spacer from the layout.
         */
        virtual void itemHoverLeaveEvent(ExtenderItem *item);

        /**
         * This function get's called for every extender when plasma exits. Implement this function
         * to store the current state of this extender (position in a layout for example), so this
         * can be restored when applet starts again. The default implementation stores the y
         * coordinate of every extender item in the config field extenderItemPos.
         */
        virtual void saveState();

        /**
         * @reimplemented from QGraphicsWidget
         */
        QVariant itemChange(GraphicsItemChange change, const QVariant &value);

        /**
         * @reimplemented from QGraphicsWidget
         */
        void resizeEvent(QGraphicsSceneResizeEvent *event);

    Q_SIGNALS:
        /**
         * Fires when an extender item is added to this extender.
         */
        void itemAttached(Plasma::ExtenderItem *);

        /**
         * Fires when an extender item is removed from this extender.
         */
        void itemDetached(Plasma::ExtenderItem *);

        /**
         * Fires when an extender's preferred size changes.
         */
        void geometryChanged();

    private:
        ExtenderPrivate *const d;

        friend class ExtenderPrivate;
        friend class ExtenderItem;
        friend class ExtenderItemPrivate;
        //applet should be able to call saveState();
        friend class Applet;

    };
} // Plasma namespace

#endif //PLASMA_EXTENDER_H

