/*
 *   Copyright (C) 2007 by Matias Valdenegro T. <mvaldenegro@informatica.utem.cl>
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

#include "hboxlayout.h"

#include <QtCore/QList>

#include <KDebug>

namespace Plasma
{

HBoxLayout::HBoxLayout(LayoutItem *parent)
    : BoxLayout(parent),
      d(0)
{
}

HBoxLayout::~HBoxLayout()
{
}

Qt::Orientations HBoxLayout::expandingDirections() const
{
    return Qt::Horizontal;
}

bool HBoxLayout::hasWidthForHeight() const
{
    return true;
}

qreal HBoxLayout::widthForHeight(qreal w) const
{
    Q_UNUSED(w);
    return qreal();
}

void HBoxLayout::setGeometry(const QRectF& geometry)
{
    if (!geometry.isValid() || geometry.isEmpty()) {
        kDebug() << "Invalid Geometry " << geometry << endl;
        return;
    }

    //kDebug() << this << " Geometry process " << geometry << " for " << children().count() << " childrens"<< endl;

    QList<LayoutItem *> fixedChildren;
    QList<LayoutItem *> expandingChildren;
    QList<QSizeF> sizes;
    QSizeF available = geometry.size() - QSizeF(2 * margin(), 2 * margin());

    foreach (LayoutItem *l, children()) {
        kDebug() << "testing layout item " << l << endl;
        if (l->expandingDirections() & Qt::Horizontal) {
            expandingChildren += l;
        } else {

            fixedChildren += l;
        }
    }

    foreach (LayoutItem *l, fixedChildren) {
        QSizeF hint = l->sizeHint();
        sizes.insert(indexOf(l), QSizeF(available.width(), hint.height()));
        available -= QSizeF(hint.width() + spacing(), 0.0f);
    }

    qreal expandWidth = (available.width() - ((expandingChildren.count() - 1) * spacing())) / expandingChildren.count();

    foreach (LayoutItem *l, expandingChildren) {

        sizes.insert(indexOf(l), QSizeF(expandWidth, available.height()));
    }

    QPointF start = QPointF(0.0f, 0.0f);
    start += QPointF(margin(), spacing());

    for (int i = 0; i < sizes.size(); i++) {
        LayoutItem *l = itemAt(i);
        kDebug() << "Setting Geometry for child " << l << " to " << QRectF(start, sizes[i]) << endl;
        l->setGeometry(QRectF(start, sizes[i]));
        start += QPointF(sizes[i].width() + spacing(), 0.0);
    }

    BoxLayout::setGeometry(geometry);
}

QSizeF HBoxLayout::sizeHint() const
{
    qreal hintHeight = 0.0;
    qreal hintWidth = 0.0;

    foreach(LayoutItem *l, children()) {

        QSizeF hint = l->sizeHint();

        hintHeight = qMax(hint.height(), hintHeight);
        hintWidth += hint.width() + spacing();
    }

    return QSizeF(hintWidth, hintHeight);
}

}
