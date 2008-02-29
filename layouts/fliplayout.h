/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Lesser General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Lesser General Public License for more details
 *
 *   You should have received a copy of the GNU Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef PLASMA_FLIPLAYOUT_H_
#define PLASMA_FLIPLAYOUT_H_

#include <QtGui/QGraphicsLayout>

#include <plasma/plasma.h>
#include <plasma/widgets/widget.h>

namespace Plasma
{

template <typename SuperLayout>
class FlipLayout : public SuperLayout { //Plasma::Layout
public:
    void setFlip(Flip flip)
    {
        m_flip = flip;
    }

    Flip flip()
    {
        return m_flip;
    }

private:
    Flip m_flip;

protected:
    void relayout()
    {
        SuperLayout::relayout();
        QRectF rect = SuperLayout::geometry();

        int count = SuperLayout::count();

        if (m_flip == NoFlip) {
            return;
        }

        QRectF childGeometry;
        for (int i = 0; i < count; i++) {
            QGraphicsLayoutItem * item = SuperLayout::itemAt(i);

            if (!item) continue;

            childGeometry = item->geometry();
            if (m_flip & HorizontalFlip) {
                // 2 * rect.left() - twice because we already have one
                // value of rect.left() inside the childGeometry.left()
                childGeometry.moveLeft(
                    2 * rect.left()       + rect.width()
                      - childGeometry.left()  - childGeometry.width()
                );
            }
            if (m_flip & VerticalFlip) {
                // 2 * rect.top() - same reason as aforemontioned
                childGeometry.moveTop(
                    2 * rect.top()      + rect.height()
                      - childGeometry.top() - childGeometry.height()
                );
            }
            item->setGeometry(childGeometry);
        }
    }

};

}

#endif /*FlipLayout_H_*/
