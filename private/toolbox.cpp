/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Marco Martin <notmart@gmail.com>
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

#include "toolbox_p.h"

#include <QAction>
#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QRadialGradient>

#include <kcolorscheme.h>
#include <kdebug.h>

#include <plasma/theme.h>
#include "widgets/iconwidget.h"

namespace Plasma
{

class ToolBoxPrivate
{
public:
    ToolBoxPrivate()
      : size(50),
      iconSize(32, 32),
      corner(ToolBox::TopRight),
      hidden(false),
      showing(false),
      movable(false),
      dragging(false)
    {}

    int size;
    QSize iconSize;
    ToolBox::Corner corner;
    QPoint dragStart;
    bool hidden : 1;
    bool showing : 1;
    bool movable : 1;
    bool dragging : 1;
};

ToolBox::ToolBox(QGraphicsItem *parent)
    : QGraphicsItem(parent),
      d(new ToolBoxPrivate)
{
    setAcceptsHoverEvents(true);
}

ToolBox::~ToolBox()
{
    delete d;
}

QPoint ToolBox::toolPosition(int toolHeight)
{
    switch (d->corner) {
    case TopRight:
        return QPoint(d->size * 2, -toolHeight);
    case Top:
        return QPoint((int)boundingRect().center().x() - d->iconSize.width(), -toolHeight);
    case TopLeft:
        return QPoint(-d->size * 2, -toolHeight);
    case Left:
        return QPoint(-d->size * 2, (int)boundingRect().center().y() - d->iconSize.height());
    case Right:
        return QPoint(d->size * 2, (int)boundingRect().center().y() - d->iconSize.height());
    case BottomLeft:
        return QPoint(-d->size * 2, toolHeight);
    case Bottom:
        return QPoint((int)boundingRect().center().x() - d->iconSize.width(), toolHeight);
    case BottomRight:
    default:
        return QPoint(d->size * 2, toolHeight);
    }
}

void ToolBox::addTool(QAction *action)
{
    if (!action) {
        return;
    }

    Plasma::IconWidget *tool = new Plasma::IconWidget(this);

    tool->setAction(action);
    tool->setDrawBackground(true);
    tool->setOrientation(Qt::Horizontal);
    tool->resize(tool->sizeFromIconSize(22));

    tool->hide();
    const int height = static_cast<int>(tool->boundingRect().height());
    tool->setPos(toolPosition(height));
    tool->setZValue(zValue() + 1);

    //make enabled/disabled tools appear/disappear instantly
    connect(tool, SIGNAL(changed()), this, SLOT(updateToolBox()));
}

void ToolBox::updateToolBox()
{
    if (d->showing) {
        d->showing = false;
        showToolBox();
    }
}

void ToolBox::removeTool(QAction *action)
{
    foreach (QGraphicsItem *child, QGraphicsItem::children()) {
        //kDebug() << "checking tool" << child << child->data(ToolName);
        Plasma::IconWidget *tool = dynamic_cast<Plasma::IconWidget*>(child);
        if (tool && tool->action() == action) {
            //kDebug() << "tool found!";
            tool->deleteLater();
            break;
        }
    }
}

int ToolBox::size() const
{
    return  d->size;
}

void ToolBox::setSize(const int newSize)
{
    d->size = newSize;
}

QSize ToolBox::iconSize() const
{
    return d->iconSize;
}

void ToolBox::setIconSize(const QSize newSize)
{
    d->iconSize = newSize;
}

bool ToolBox::showing() const
{
    return  d->showing;
}

void ToolBox::setShowing(const bool show)
{
    d->showing = show;
}

void ToolBox::setCorner(const Corner corner)
{
    d->corner = corner;
}

ToolBox::Corner ToolBox::corner() const
{
    return d->corner;
}

void ToolBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
    d->dragStart = mapToParent(event->pos()).toPoint();
}

void ToolBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->movable || (!d->dragging && boundingRect().contains(event->pos()))) {
        return;
    }

    //TODOs:
    // save toolbox position
    // move relative to where on the toolbox it was grabbed
    // stop the toolbox from painting and having a boundingRect that is negative
    // sticky points at midpoints
    // change how buttons appear depending on the location of the box
    d->dragging = true;
    const QPoint newPos = mapToParent(event->pos()).toPoint();
    const QPoint curPos = pos().toPoint();
    const int h = abs(boundingRect().height());
    const int w = abs(boundingRect().width());

    const int areaWidth = parentWidget()->size().width();
    const int areaHeight = parentWidget()->size().height();

    int x = curPos.x();
    int y = curPos.y();

    if (y == 0 || y + h >= areaHeight) {
        x = curPos.x() + (newPos.x() - d->dragStart.x());
        if (x < 0) {
            x = 0;
        } else if (x + w > areaWidth) {
            x = areaWidth - w;
        }
    }

    //kDebug() << x << w << areaWidth;
    if (x == 0 || x + w >= areaWidth) {
        //kDebug() << "moving along the y axis" << curPos << newPos << d->dragStart;
        y = curPos.y() + (newPos.y() - d->dragStart.y());

        if (y < 0) {
            y = 0;
        } else if (y + h > areaHeight) {
            y = areaHeight - h;
        }
    }

    x = qBound(0, x, areaWidth - w);
    y = qBound(0, y, areaHeight - h);


    Corner newCorner = d->corner;
    if (x == 0) {
        if (y == 0) {
            newCorner = TopLeft;
        } else if (y + h >= areaHeight) {
            newCorner = BottomLeft;
        } else {
            newCorner = Left;
        }
    } else if (y == 0) {
        if (x + w >= areaWidth) {
            newCorner = TopRight;
        } else {
            newCorner = Top;
        }
    } else if (x + w >= areaWidth) {
        if (y + h >= areaHeight) {
            newCorner = BottomRight;
        } else {
            newCorner = Right;
        }
    } else {
        newCorner = Bottom;
    }

    if (newCorner != d->corner) {
        prepareGeometryChange();
        d->corner = newCorner;
    }

    setPos(x, y);
    d->dragStart = newPos;
}

void ToolBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->dragging && boundingRect().contains(event->pos())) {
        emit toggled();
    }

    d->dragStart = QPoint();
    d->dragging = false;
}

bool ToolBox::isMovable() const
{
    return d->movable;
}

void ToolBox::setIsMovable(bool movable)
{
    d->movable = movable;
}

} // plasma namespace

#include "toolbox_p.moc"

