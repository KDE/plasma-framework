/*
 * Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
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

#ifndef PLASMA_EXTENDERGROUP_H
#define PLASMA_EXTENDERGROUP_H

#include <QtGui/QGraphicsWidget>
#include <QtCore/QList>

#include "extenderitem.h"

#include "plasma/plasma_export.h"

namespace Plasma
{

class ExtenderGroupPrivate;
class ExtenderItem;
class Applet;

/**
 * @class ExtenderGroup plasma/extendergroup.h <Plasma/ExtenderGroup>
 *
 * @short Allows for grouping of extender items.
 *
 * To be able to group multiple items together, you'll need to instantiate an ExtenderGroup, and
 * call setGroup() on all extender items you wish to add to this group.
 * This ExtenderGroup is just the same as any other ExtenderItem, except for the expand group and
 * collapse group buttons it provides, and the fact that it will automatically hide itself if less
 * then one item belong to this group and autoHide is set to true.
 *
 * @since 4.3
 */
class PLASMA_EXPORT ExtenderGroup : public ExtenderItem
{
    Q_OBJECT
    Q_PROPERTY(bool autoHide READ autoHide WRITE setAutoHide)
    Q_PROPERTY(bool groupCollapsed READ isGroupCollapsed WRITE setGroupCollapsed)
    Q_PROPERTY(bool autoCollapse READ isAutoCollapse WRITE setAutoCollapse)

    public:
        /**
         * Creates a group.
         * @param applet The applet this group is part of. Null is not allowed here.
         */
        explicit ExtenderGroup(Extender *parent, uint groupId = 0);

        ~ExtenderGroup();

        /**
         * @return a list of items that belong to this group.
         */
        QList<ExtenderItem*> items() const;

        /**
         * @return whether or not this item hides itself if there are less then 2 items in.
         */
        bool autoHide() const;

        /**
         * @param autoHide whether or not this item hides itself if less then 2 items belong to this group. The default value is true.
         */
        void setAutoHide(bool autoHide);

        /**
         * @return if the group is collapsed
         * @since 4.4
         */
        bool isGroupCollapsed() const;

        /**
         * @return whether or not this item collapses itself when the group gets collapsed
         * @since 4.4
         */
        bool isAutoCollapse() const;

        /**
         * @param autoCollapse whether or not this item collapses itself when the group gets collapsed, the default value is false
         * @since 4.4
         */
        void setAutoCollapse(bool collapse);

    public Q_SLOTS:
        /**
         * expands or collapses this group
         * @since 4.4
         */
        void setGroupCollapsed(bool collapsed);

        /**
         * Expands this group to show all ExtenderItems that are contained in this group.
         */
        void expandGroup();

        /**
         * Collapses this group to hide all ExtenderItems that are contained in this group, and
         * shows the summary item.
         */
        void collapseGroup();

    private:
        ExtenderGroupPrivate * const d;

        Q_PRIVATE_SLOT(d, void addItemToGroup(Plasma::ExtenderItem *item))
        Q_PRIVATE_SLOT(d, void removeItemFromGroup(Plasma::ExtenderItem *item))
        Q_PRIVATE_SLOT(d, void themeChanged())

        friend class ExtenderItem;
};
} // Plasma namespace

#endif //PLASMA_EXTENDER_H

