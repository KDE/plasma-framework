/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2010 Davide Bettio <davide.bettio@kdemail.net>
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

#ifndef ANIMABLEWEBVIEW_P_H
#define ANIMABLEWEBVIEW_P_H

#include <QtGui/QGraphicsSceneContextMenuEvent>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneWheelEvent>
#include <QtGui/QGraphicsView>

#include <kgraphicswebview.h>

namespace Plasma
{

class AnimableGraphicsWebView : public KGraphicsWebView
{
    Q_OBJECT

    Q_PROPERTY(QPointF scrollPosition READ scrollPosition WRITE setScrollPosition)
    Q_PROPERTY(QSizeF contentsSize READ contentsSize)
    Q_PROPERTY(QRectF viewportGeometry READ viewportGeometry)

public:
    AnimableGraphicsWebView(QGraphicsItem * parent = 0);

    QPointF scrollPosition() const;
    void setScrollPosition(const QPointF &position);
    QSizeF contentsSize() const;
    QRectF viewportGeometry() const;
    void setDragToScroll(bool enable);
    bool dragToScroll() const;

protected:
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);
    void mousePressEvent(QGraphicsSceneMouseEvent *event);
    void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
    void wheelEvent(QGraphicsSceneWheelEvent *event);

private:
    bool m_dragToScroll;
    QPointF m_lastScrollPosition;
};

}

#endif
