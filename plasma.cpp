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

#include <QAction>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QMenu>

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
    switch (location) {
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

Direction locationToInverseDirection(Location location)
{
    switch (location) {
    case Floating:
    case Desktop:
    case TopEdge:
    case FullScreen:
        //TODO: should we be smarter for floating and planer?
        //      perhaps we should take a QRect and/or QPos as well?
        return Up;
    case BottomEdge:
        return Down;
    case LeftEdge:
        return Left;
    case RightEdge:
        return Right;
    }

    return Up;
}

QGraphicsView *viewFor(const QGraphicsItem *item)
{
    if (!item || !item->scene()) {
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

QList<QAction*> actionsFromMenu(QMenu *menu, const QString &prefix, QObject *parent)
{
    Q_ASSERT(menu);

    QList<QAction*> ret;
    foreach (QAction *action, menu->actions()) {
        if (QMenu *submenu = action->menu()) {
            //Flatten hierarchy and prefix submenu text to all actions in submenu
            ret << actionsFromMenu(submenu, action->text(), parent);
        } else if (!action->isSeparator() && action->isEnabled()) {
            QString text = action->text();
            if (action->isCheckable()) {
                if (action->isChecked()) {
                    text = QString("(%1) %2").arg(QChar(0x2613)).arg(text);
                } else {
                    text = QString("( ) %1").arg(text);
                }
            }

            if (!prefix.isEmpty()) {
                text = QString("%1: %2").arg(prefix).arg(text);
            }
            text = text.replace(QRegExp("&([\\S])"), "\\1");

            QAction *a = new QAction(action->icon(), text, parent);

            QObject::connect(a, SIGNAL(triggered(bool)), action, SIGNAL(triggered(bool)));
            ret << a;
        }
    }
    return ret;
}

} // Plasma namespace
