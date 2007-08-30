/*
 *   Copyright 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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
#include <plasma/widgets/layout.h>

namespace Plasma
{


/**
 * Vertical Box Layout
 *
 * @author Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
 *
 * This class implements a generic box Layout used as a common API for VBox and HBox implementations.
 */
class PLASMA_EXPORT BoxLayout : public Layout
{
    public:

        /**
         * Constructor.
        */
        explicit BoxLayout(Qt::Orientations orientation, LayoutItem *parent = 0);

        /**
         * Destructor.
        */
        ~BoxLayout();

        Qt::Orientations expandingDirections() const;

        QSizeF minimumSize() const;
        QSizeF maximumSize() const;

        QRectF geometry() const;
        void setGeometry(const QRectF& geometry);

        QSizeF sizeHint() const;

        int count() const;

        bool isEmpty() const;

        void insertItem(int index, LayoutItem *l);
        void addItem(LayoutItem *l);

        void removeItem(LayoutItem *l);

        int indexOf(LayoutItem *l) const;
        LayoutItem *itemAt(int i) const;
        LayoutItem *takeAt(int i);

        QSizeF size() const;

    protected:
        QList<LayoutItem *> children() const;

    private:
        class Private;
        Private *const d;
};

}

#endif /* _H_BOX_LAYOUT__ */
