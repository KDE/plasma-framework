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

#ifndef PLASMA_EXTENDERGROUPPRIVATE_H
#define PLASMA_EXTENDERGROUPPRIVATE_H

namespace Plasma 
{

class ScrollWidget;
class ExtenderGroup;
class ExtenderItem;
class Svg;
class Spacer;

class ExtenderGroupPrivate
{
    public:
        ExtenderGroupPrivate(ExtenderGroup *group);
        ~ExtenderGroupPrivate();

        void addItemToGroup(Plasma::ExtenderItem *item, const QPointF &pos = QPointF(-1, -1));
        void removeItemFromGroup(Plasma::ExtenderItem *item);
        void themeChanged();
        int insertIndexFromPos(const QPointF &pos) const;

        Plasma::ExtenderGroup *q;
        Spacer *spacerWidget;
        Plasma::Svg *svg;
        Plasma::ScrollWidget *scrollWidget;
        QGraphicsWidget *childsWidget;
        QGraphicsLinearLayout *layout;
        bool collapsed;
        bool autoHide;
        bool autoCollapse;
};
}

#endif
