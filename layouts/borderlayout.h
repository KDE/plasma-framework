/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_BORDER_LAYOUT
#define PLASMA_BORDER_LAYOUT

#include <QtCore/QMap>

#include <plasma/plasma_export.h>
#include <plasma/plasma.h>
#include <plasma/layouts/layout.h>

namespace Plasma {

class BorderLayoutPrivate;

/**
 * A layout which lays one item per border (left, top, bottom, right)
 * and one item in center.
 *
 * Similar to java.awt.BorderLayout from the Java's standard library
 */
class PLASMA_EXPORT BorderLayout : public Layout {
public:

    explicit BorderLayout(LayoutItem * parent = 0);
    virtual ~BorderLayout();

    virtual Qt::Orientations expandingDirections() const;

    QSizeF sizeHint() const;

    /**
     * Adds item in the center. Equal to:
     * addItem(item, Plasma::CenterPositioned);
     */
    void addItem(Plasma::LayoutItem * item);

    /**
     * Adds item at the specified position
     */
    void addItem(Plasma::LayoutItem * item, Position position);

    void removeItem(Plasma::LayoutItem * item);

    virtual int count() const;
    virtual int indexOf(LayoutItem * item) const;
    virtual LayoutItem * itemAt(int i) const;
    virtual LayoutItem * takeAt(int i);

    /**
     * Deactivates the automatic sizing of a border widget,
     * and sets it to the specified size.
     *
     * For left and right widgets, it sets the width; while
     * for top and bottom ones, it sets the height.
     */
    void setSize(qreal size, Position border);

    /**
     * Activates the automatic sizing of a border widget,
     * according to it's sizeHint()
     */
    void setAutoSize(Position border);

    /**
     * Returns the size of the specified border widget. If
     * automatic sizing for that border widget is activated,
     * it will return a value less than zero.
     */
    qreal size(Position border);

protected:
    void relayout();
    void releaseManagedItems();

private:
    BorderLayoutPrivate * const d;
};

}

#endif /* PLASMA_BORDER_LAYOUT */
