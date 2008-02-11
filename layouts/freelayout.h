/*
 *   Copyright 2007 by Robert Knight <robertknight@gmail.com>
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

#ifndef PLASMA_FREE_LAYOUT_H
#define PLASMA_FREE_LAYOUT_H

#include <QtCore/QList>

#include <plasma/plasma_export.h>
#include <plasma/layouts/layout.h>

namespace Plasma
{

/**
 * The FreeLayout class is a layout for use in desktop-like workspaces where
 * items can be moved around freely and applets and widgets are allowed
 * to determine their own size.
 *
 * Whenever this layout is updated, all child items are resized to
 * their sizeHint() and left in their existing positions.
 */
class PLASMA_EXPORT FreeLayout : public Layout
{
    public:
        /**
         * Creates a new free layout
         */
        explicit FreeLayout(LayoutItem *parent = 0);
        ~FreeLayout();

        // reimplemented from Layout
        virtual void addItem(LayoutItem *l);
        virtual void removeItem(LayoutItem *l);
        virtual int indexOf(LayoutItem *l) const;
        virtual LayoutItem *itemAt(int i) const;
        virtual LayoutItem *takeAt(int i);
        virtual Qt::Orientations expandingDirections() const;
        virtual QRectF geometry() const;
        virtual int count() const;

        virtual QSizeF sizeHint() const;

    protected:
        void relayout();
        void releaseManagedItems();

    private:
        class Private;
        Private *const d;
};

}

#endif /* PLASMA_FREE_LAYOUT_H */
