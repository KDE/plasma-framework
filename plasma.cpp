/*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
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

#include <plasma/plasma.h>

#include <QDesktopWidget>
#include <QGraphicsScene>
#include <QGraphicsView>

#include <plasma/containment.h>
#include <plasma/view.h>

namespace Plasma
{

qreal scalingFactor(ZoomLevel level)
{
    switch (level) {
    case DesktopZoom:
        return 1;
        break;
    case GroupZoom:
        return 0.5;
        break;
    case OverviewZoom:
        return 0.2;
        break;
    }

    // to make odd compilers not warn like silly beasts
    return 1;
}

Direction locationToDirection(Location location)
{
    switch (location)
    {
        case Floating:
        case Desktop:
        case TopEdge:
        case FullScreen:
            //TODO: should we be smarter for floating and planer?
            //      perhaps we should take a QRect and/or QPos as well?
            return Down;
        case BottomEdge:
            return Up;
        case LeftEdge:
            return Right;
        case RightEdge:
            return Left;
    }

    return Down;
}

QPoint popupPosition(const QGraphicsItem * item, const QSize &s)
{
    QGraphicsView *v = viewFor(item);

    if (!v) {
        return QPoint(0,0);
    }

    QPoint pos = v->mapFromScene(item->scenePos());
    pos = v->mapToGlobal(pos);
    //kDebug() << "==> position is" << scenePos() << v->mapFromScene(scenePos()) << pos;
    Plasma::View *pv = dynamic_cast<Plasma::View *>(v);

    Plasma::Location loc = Floating;
    if (pv) {
        loc = pv->containment()->location();
    }

    switch (loc) {
    case BottomEdge:
        pos = QPoint(pos.x(), pos.y() - s.height());
        break;
    case TopEdge:
        pos = QPoint(pos.x(), pos.y() + (int)item->boundingRect().size().height());
        break;
    case LeftEdge:
        pos = QPoint(pos.x() + (int)item->boundingRect().size().width(), pos.y());
        break;
    case RightEdge:
        pos = QPoint(pos.x() - s.width(), pos.y());
        break;
    default:
        if (pos.y() - s.height() > 0) {
             pos = QPoint(pos.x(), pos.y() - s.height());
        } else {
             pos = QPoint(pos.x(), pos.y() + (int)item->boundingRect().size().height());
        }
    }

    //are we out of screen?
    QRect screenRect = QApplication::desktop()->screenGeometry(pv ? pv->containment()->screen() : -1);
    //kDebug() << "==> rect for" << (pv ? pv->containment()->screen() : -1) << "is" << screenRect;

    if (pos.rx() + s.width() > screenRect.right()) {
        pos.rx() -= ((pos.rx() + s.width()) - screenRect.right());
    }

    if (pos.ry() + s.height() > screenRect.bottom()) {
        pos.ry() -= ((pos.ry() + s.height()) - screenRect.bottom());
    }

    pos.rx() = qMax(0, pos.rx());
    return pos;
}

QGraphicsView* viewFor(const QGraphicsItem * item)
{
    if (!item->scene()) {
        return 0;
    }

    QGraphicsView *found = 0;
    foreach (QGraphicsView *view, item->scene()->views()) {
        if (view->sceneRect().intersects(item->sceneBoundingRect()) ||
            view->sceneRect().contains(item->scenePos())) {
            if (!found || view->isActiveWindow()) {
                found = view;
            }
        }
    }

    return found;
}

} // Plasma namespace
