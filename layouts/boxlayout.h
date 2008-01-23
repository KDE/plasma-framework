/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
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

#ifndef PLASMA_BOX_LAYOUT
#define PLASMA_BOX_LAYOUT

#include <QtCore/QList>

#include <plasma/plasma_export.h>
#include <plasma/layouts/layout.h>

namespace Plasma
{

/**
 * The BoxLayout class lays out items in a horizontal or vertical line.
 */
class PLASMA_EXPORT BoxLayout : public Layout
{
    public:
        /** 
         * This enum describes the directions in which items can be laid 
         * out.
         */
        enum Direction
        {
            /** Lay items out horizontally, from left to right. */
            LeftToRight,
            /** Lay items out horizontally, from right to left. */
            RightToLeft,
            /** Lay items out vertically, from top to bottom. */
            TopToBottom,
            /** Lay items out vertically, from bottom to top. */
            BottomToTop
        };

        /**
         * Creates a new box layout which lays items out in the specified
         * @p direction
         */
        explicit BoxLayout(Direction direction, LayoutItem *parent = 0);
        ~BoxLayout();

        /** Sets the direction in which items are laid out. */
        void setDirection(Direction direction);
        /** Returns the direction in which items are laid out. */
        Direction direction() const;

        /** Inserts a new item into the layout at the specified index. */
        void insertItem(int index, LayoutItem *l);

        /** Set weither this layout will take severals row */
        void setMultiRow(bool b);

        // reimplemented from Layout
        virtual void addItem(LayoutItem *l);
        virtual void removeItem(LayoutItem *l);
        virtual int indexOf(LayoutItem *l) const;
        virtual LayoutItem *itemAt(int i) const;
        virtual LayoutItem *takeAt(int i);
        virtual Qt::Orientations expandingDirections() const;
        virtual int count() const;
        virtual void setAnimator(LayoutAnimator* animator);

        virtual QSizeF minimumSize() const;
        virtual QSizeF maximumSize() const;
        virtual QSizeF sizeHint() const;

    protected:
        void relayout();

    private:
        class Private;
        Private *const d;
};

/** 
 * A BoxLayout which defaults to laying items out 
 * horizontally in a left-to-right order.
 *
 * Equivalent to creating a BoxLayout and passing LeftToRight
 * in the constructor.
 */
class PLASMA_EXPORT HBoxLayout : public BoxLayout 
{
public:
    explicit HBoxLayout(LayoutItem *parent = 0);
};

/**
 * A BoxLayout which defaults to laying items out
 * vertically in a top-to-bottom order.
 *
 * Equivalent to creating a BoxLayout and passing TopToBottom
 * in the constructor.
 */
class PLASMA_EXPORT VBoxLayout : public BoxLayout 
{
public:
    explicit VBoxLayout(LayoutItem *parent = 0);
};

}

#endif /* PLASMA_BOX_LAYOUT */
