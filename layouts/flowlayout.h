/*
*   Copyright 2007 by Robert Knight <robertknight@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License
*   as published by the Free Software Foundation; either
*   version 2 of the License, or (at your option) any later version.
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

#ifndef __FLOWLAYOUT__
#define __FLOWLAYOUT__

#include <plasma/plasma_export.h>
#include <plasma/layouts/layout.h>

namespace Plasma
{

/**
 * A layout which lays items out left-to-right , top-to-bottom.
 *
 * This is similar to the layout of items in a QListView. 
 */
class PLASMA_EXPORT FlowLayout : public Layout
{
public:
    /** Construct a new flow layout with the specified parent. */
    explicit FlowLayout(LayoutItem* parent);
    virtual ~FlowLayout();

    // reimplemented
    virtual int count() const;
    virtual void addItem(LayoutItem* item);
    virtual void removeItem(LayoutItem* item);
    virtual int indexOf(LayoutItem* item) const;
    virtual LayoutItem* itemAt(int i) const;
    virtual LayoutItem* takeAt(int i);

    virtual QSizeF sizeHint() const;
    virtual Qt::Orientations expandingDirections() const;
    virtual void setColumnWidth( const qreal width );
    virtual qreal columnWidth() const;

protected:
    void relayout();
    void releaseManagedItems();

private:
    class Private;
    Private *const d;
};

}

#endif // __FLOWLAYOUT__

