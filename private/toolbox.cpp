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
#include <QApplication>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QRadialGradient>

#include <kcolorscheme.h>
#include <kconfiggroup.h>
#include <kdebug.h>

#include "corona.h"
#include "theme.h"
#include "widgets/iconwidget.h"

namespace Plasma
{

class ToolBoxPrivate
{
public:
    ToolBoxPrivate(Containment *c)
      : containment(c),
        size(50),
        iconSize(32, 32),
        corner(ToolBox::TopRight),
        hidden(false),
        showing(false),
        movable(false),
        toolbar(false),
        dragging(false),
        userMoved(false)
    {}

    Containment *containment;
    int size;
    QSize iconSize;
    ToolBox::Corner corner;
    QPoint dragStart;
    QTransform viewTransform;
    bool hidden : 1;
    bool showing : 1;
    bool movable : 1;
    bool toolbar : 1;
    bool dragging : 1;
    bool userMoved : 1;
};

ToolBox::ToolBox(Containment *parent)
    : QGraphicsItem(parent),
      d(new ToolBoxPrivate(parent))
{
    d->userMoved = false;
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
        return QPoint(d->size, -toolHeight);
    case Top:
        return QPoint((int)boundingRect().center().x() - d->iconSize.width(), -toolHeight);
    case TopLeft:
        return QPoint(-d->size, -toolHeight);
    case Left:
        return QPoint(-d->size, (int)boundingRect().center().y() - d->iconSize.height());
    case Right:
        return QPoint(d->size, (int)boundingRect().center().y() - d->iconSize.height());
    case BottomLeft:
        return QPoint(-d->size, toolHeight);
    case Bottom:
        return QPoint((int)boundingRect().center().x() - d->iconSize.width(), toolHeight);
    case BottomRight:
    default:
        return QPoint(d->size, toolHeight);
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
    //kDebug() << "added tool" << action->text() << (QGraphicsItem*)tool;
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
            delete tool;
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
    if (!d->movable || (!d->dragging && boundingRect().contains(event->pos())) || isToolbar()) {
        return;
    }

    //TODOs:
    // move relative to where on the toolbox it was grabbed
    // sticky points at midpoints
    d->dragging = true;
    d->userMoved = true;
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
    KConfigGroup cg(d->containment->config());
    save(cg);
}

bool ToolBox::isMovable() const
{
    return d->movable;
}

void ToolBox::setIsMovable(bool movable)
{
    d->movable = movable;
}

bool ToolBox::isToolbar() const
{
    return d->toolbar;
}

void ToolBox::setIsToolbar(bool toolbar)
{
    d->toolbar = toolbar;
}

QTransform ToolBox::viewTransform() const
{
    return d->viewTransform;
}

void ToolBox::setViewTransform(QTransform transform)
{
    d->viewTransform = transform;
    if (transform.isScaling()) {
        d->toolbar = true;
        showToolBox();
    } else {
        d->toolbar = false;
    }
}

void ToolBox::save(KConfigGroup &cg) const
{
    if (!d->movable) {
        return;
    }

    KConfigGroup group(&cg, "ToolBox");
    if (!d->userMoved) {
        group.deleteGroup();
        return;
    }

    int offset = 0;
    if (d->corner == ToolBox::Left ||
        d->corner == ToolBox::Right) {
        offset = y();
    } else if (d->corner == ToolBox::Left ||
               d->corner == ToolBox::Right) {
        offset = x();
    }

    group.writeEntry("corner", int(d->corner));
    group.writeEntry("offset", offset);
}

void ToolBox::load()
{
    if (!d->movable) {
        return;
    }

    KConfigGroup group = d->containment->config();
    group = KConfigGroup(&group, "ToolBox");

    if (!group.hasKey("corner")) {
        return;
    }

    d->userMoved = true;
    d->corner = Corner(group.readEntry("corner", int(d->corner)));

    int offset = group.readEntry("offset", 0);
    switch (d->corner) {
        case ToolBox::TopLeft:
            setPos(0, 0);
            break;
        case ToolBox::Top:
            setPos(offset, 0);
            break;
        case ToolBox::TopRight:
            setPos(d->containment->size().width() - d->size, 0);
            break;
        case ToolBox::Right:
            setPos(d->containment->size().width() - d->size, offset);
            break;
        case ToolBox::BottomRight:
            setPos(d->containment->size().width() - d->size, d->containment->size().height() - d->size);
            break;
        case ToolBox::Bottom:
            setPos(offset, d->containment->size().height() - d->size);
            break;
        case ToolBox::BottomLeft:
            setPos(0, d->containment->size().height() - d->size);
            break;
        case ToolBox::Left:
            setPos(0, offset);
            break;
    }
    //kDebug() << "marked as user moved" << pos()
    //         << (d->containment->containmentType() == Containment::PanelContainment);
}

void ToolBox::reposition()
{
    if (d->userMoved) {
        //FIXME: adjust for situations like changing of the available space
        load();
        return;
    }

    if (d->containment->containmentType() == Containment::PanelContainment) {
        QRectF rect = boundingRect();
        if (d->containment->formFactor() == Vertical) {
            setCorner(ToolBox::Bottom);
            setPos(d->containment->geometry().width() / 2 - rect.width() / 2,
                   d->containment->geometry().height() - rect.height());
        } else {
            //defaulting to Horizontal right now
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                setPos(d->containment->geometry().left(),
                       d->containment->geometry().height() / 2 - rect.height() / 2);
                setCorner(ToolBox::Left);
            } else {
                setPos(d->containment->geometry().width() - rect.width(),
                       d->containment->geometry().height() / 2 - rect.height() / 2);
                setCorner(ToolBox::Right);
            }
        }
        //kDebug() << pos();
    } else if (d->containment->corona()) {
        //kDebug() << "desktop";

        int screen = d->containment->screen();
        QRectF avail = d->containment->geometry();
        QRectF screenGeom = avail;

        if (screen > -1 && screen < d->containment->corona()->numScreens()) {
            avail = d->containment->corona()->availableScreenRegion(screen).boundingRect();
            screenGeom = d->containment->corona()->screenGeometry(screen);
            avail.translate(-screenGeom.topLeft());
        }

        // Transform to the containment's coordinate system.
        screenGeom.moveTo(0, 0);

        if (!d->containment->view() || !d->containment->view()->transform().isScaling()) {
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                if (avail.top() > screenGeom.top()) {
                    setPos(avail.topLeft() - QPoint(0, d->size));
                    setCorner(ToolBox::Left);
                } else if (avail.left() > screenGeom.left()) {
                    setPos(avail.topLeft() - QPoint(d->size, 0));
                    setCorner(ToolBox::Top);
                } else {
                    setPos(avail.topLeft());
                    setCorner(ToolBox::TopLeft);
                }
            } else {
                if (avail.top() > screenGeom.top()) {
                    setPos(avail.topRight() - QPoint(0, d->size));
                    setCorner(ToolBox::Right);
                } else if (avail.right() < screenGeom.right()) {
                    setPos(avail.topRight() - QPoint(d->size, 0));
                    setCorner(ToolBox::Top);
                } else {
                    setPos(avail.topRight() - QPoint(d->size, 0));
                    setCorner(ToolBox::TopRight);
                }
            }
        } else {
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                setPos(d->containment->mapFromScene(QPointF(d->containment->geometry().topLeft())));
                setCorner(ToolBox::TopLeft);
            } else {
                setPos(d->containment->mapFromScene(QPointF(d->containment->geometry().topRight())));
                setCorner(ToolBox::TopRight);
            }
        }
    }
}

} // plasma namespace

#include "toolbox_p.moc"

