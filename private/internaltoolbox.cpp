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

#include "internaltoolbox_p.h"

#include <QAction>
#include <QApplication>
#include <QGraphicsSceneHoverEvent>
#include <QGraphicsView>
#include <QPainter>
#include <QRadialGradient>

#include <kcolorscheme.h>
#include <kconfiggroup.h>
#include <kiconloader.h>
#include <kdebug.h>

#include "corona.h"
#include "theme.h"
#include "widgets/iconwidget.h"

namespace Plasma
{

class InternalToolBoxPrivate
{
public:
    InternalToolBoxPrivate(Containment *c)
      : containment(c),
        corner(InternalToolBox::TopRight),
        size(50),
        iconSize(KIconLoader::SizeMedium, KIconLoader::SizeMedium),
        hidden(false),
        showing(false),
        movable(false),
        dragging(false),
        userMoved(false),
        iconic(true)
    {}

    Containment *containment;
    InternalToolBox::Corner corner;
    int size;
    QSize iconSize;
    QPoint dragStartRelative;
    QTransform viewTransform;
    QList<QAction *> actions;
    bool hidden : 1;
    bool showing : 1;
    bool movable : 1;
    bool dragging : 1;
    bool userMoved : 1;
    bool iconic : 1;
};

InternalToolBox::InternalToolBox(Containment *parent)
    : AbstractToolBox(parent),
      d(new InternalToolBoxPrivate(parent))
{
    d->userMoved = false;
    setAcceptsHoverEvents(true);
}

InternalToolBox::~InternalToolBox()
{
    delete d;
}

Containment *InternalToolBox::containment()
{
    return d->containment;
}

QPoint InternalToolBox::toolPosition(int toolHeight)
{
    switch (corner()) {
    case TopRight:
        return QPoint(boundingRect().width(), -toolHeight);
    case Top:
        return QPoint((int)boundingRect().center().x() - boundingRect().width(), -toolHeight);
    case TopLeft:
        return QPoint(-boundingRect().width(), -toolHeight);
    case Left:
        return QPoint(-boundingRect().width(), (int)boundingRect().center().y() - boundingRect().height());
    case Right:
        return QPoint(boundingRect().width(), (int)boundingRect().center().y() - boundingRect().height());
    case BottomLeft:
        return QPoint(-boundingRect().width(), toolHeight);
    case Bottom:
        return QPoint((int)boundingRect().center().x() - d->iconSize.width(), toolHeight);
    case BottomRight:
    default:
        return QPoint(boundingRect().width(), toolHeight);
    }
}

QGraphicsWidget *InternalToolBox::toolParent()
{
    return this;
}

QList<QAction *> InternalToolBox::actions() const
{
    return d->actions;
}

void InternalToolBox::addTool(QAction *action)
{
    if (!action) {
        return;
    }

    if (d->actions.contains(action)) {
        return;
    }

    connect(action, SIGNAL(destroyed(QObject*)), this, SLOT(actionDestroyed(QObject*)));
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toolTriggered(bool)));
    d->actions.append(action);
}

void InternalToolBox::removeTool(QAction *action)
{
    disconnect(action, 0, this, 0);
    d->actions.removeAll(action);
}

void InternalToolBox::actionDestroyed(QObject *object)
{
    d->actions.removeAll(static_cast<QAction*>(object));
}

bool InternalToolBox::isEmpty() const
{
    return d->actions.isEmpty();
}

void InternalToolBox::toolTriggered(bool)
{
}

int InternalToolBox::size() const
{
    return  d->size;
}

void InternalToolBox::setSize(const int newSize)
{
    d->size = newSize;
}

QSize InternalToolBox::iconSize() const
{
    return d->iconSize;
}

void InternalToolBox::setIconSize(const QSize newSize)
{
    d->iconSize = newSize;
}

bool InternalToolBox::isShowing() const
{
    return d->showing;
}

void InternalToolBox::setShowing(const bool show)
{
    if (show) {
        showToolBox();
    } else {
        hideToolBox();
    }
    d->showing = show;
}

void InternalToolBox::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        event->accept();
        // set grab position relative to toolbox
        d->dragStartRelative = mapToParent(event->pos()).toPoint() - pos().toPoint();
    } else {
        event->ignore();
    }
}

QSize InternalToolBox::cornerSize() const
{
    return boundingRect().size().toSize();
}

QSize InternalToolBox::fullWidth() const
{
    return boundingRect().size().toSize();
}

QSize  InternalToolBox::fullHeight() const
{
    return boundingRect().size().toSize();
}

void InternalToolBox::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->movable || (!d->dragging && boundingRect().contains(event->pos()))) {
        return;
    }

    d->dragging = true;
    d->userMoved = true;
    const QPoint newPos = mapToParent(event->pos()).toPoint();
    const QPoint curPos = pos().toPoint();

    const QSize cSize = cornerSize();
    const QSize fHeight = fullHeight();
    const QSize fWidth = fullWidth();
    const int h = fHeight.height();
    const int w = fWidth.width();

    const int areaWidth = parentWidget()->size().width();
    const int areaHeight = parentWidget()->size().height();

    int x = curPos.x();
    int y = curPos.y();

    // jump to the nearest desktop border
    int distanceToLeft = newPos.x() - d->dragStartRelative.x();
    int distanceToRight = areaWidth - w - distanceToLeft;
    int distanceToTop = newPos.y() - d->dragStartRelative.y();
    int distanceToBottom = areaHeight - h - distanceToTop;

    int distancetoHorizontalMiddle = qAbs((newPos.x() + boundingRect().size().width()/2) - areaWidth/2 - d->dragStartRelative.x());
    int distancetoVerticalMiddle = qAbs((newPos.y() + boundingRect().size().height()/2) - areaHeight/2 - d->dragStartRelative.y());

    if (distancetoHorizontalMiddle < 10) {
        x = areaWidth/2 - boundingRect().size().width()/2;
    } else if (distancetoVerticalMiddle < 10) {
        y = areaHeight/2 - boundingRect().size().height()/2;
    } else {
        // decide which border is the nearest
        if (distanceToLeft < distanceToTop && distanceToLeft < distanceToRight &&
            distanceToLeft < distanceToBottom ) {
            x = 0;
            y = (newPos.y() - d->dragStartRelative.y());
        } else if (distanceToRight < distanceToTop && distanceToRight < distanceToLeft &&
                distanceToRight < distanceToBottom) {
            x = areaWidth - w;
            y = (newPos.y() - d->dragStartRelative.y());
        } else if (distanceToTop < distanceToLeft && distanceToTop < distanceToRight &&
                distanceToTop < distanceToBottom ) {
            y = 0;
            x = (newPos.x() - d->dragStartRelative.x());
        } else if (distanceToBottom < distanceToLeft && distanceToBottom < distanceToRight &&
                distanceToBottom < distanceToTop) {
            y = areaHeight - h;
            x = (newPos.x() - d->dragStartRelative.x());
        }
    }


    x = qBound(0, x, areaWidth - w);
    y = qBound(0, y, areaHeight - h);

    Corner newCorner = corner();
    if (x == 0) {
        if (y == 0) {
            newCorner = TopLeft;
        } else if (areaHeight - cSize.height() < newPos.y()) {
            y = areaHeight - cSize.height();
            newCorner = BottomLeft;
        } else {
            newCorner = Left;
        }
    } else if (y == 0) {
        if (areaWidth - cSize.width() < newPos.x()) {
            x = areaWidth - cSize.width();
            newCorner = TopRight;
        } else {
            newCorner = Top;
        }
    } else if (x + w >= areaWidth) {
        if (areaHeight - cSize.height() < newPos.y()) {
            y = areaHeight - cSize.height();
            x = areaWidth - cSize.width();
            newCorner = BottomRight;
        } else {
            x = areaWidth - fHeight.width();
            newCorner = Right;
        }
    } else {
        y = areaHeight - fWidth.height();
        newCorner = Bottom;
    }

    if (newCorner != corner()) {
        prepareGeometryChange();
        setCorner(newCorner);
    }

    setPos(x, y);
}

void InternalToolBox::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && !d->dragging && boundingRect().contains(event->pos())) {
        emit toggled();
        return;
    }

    d->dragging = false;
    KConfigGroup cg(d->containment->config());
    save(cg);
}

bool InternalToolBox::isMovable() const
{
    return d->movable;
}

void InternalToolBox::setIsMovable(bool movable)
{
    d->movable = movable;
}

void InternalToolBox::setCorner(const Corner corner)
{
    d->corner = corner;
}

InternalToolBox::Corner InternalToolBox::corner() const
{
    return d->corner;
}

void InternalToolBox::save(KConfigGroup &cg) const
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
    if (corner() == InternalToolBox::Left ||
        corner() == InternalToolBox::Right) {
        offset = y();
    } else if (corner() == InternalToolBox::Top ||
               corner() == InternalToolBox::Bottom) {
        offset = x();
    }

    group.writeEntry("corner", int(corner()));
    group.writeEntry("offset", offset);
}

void InternalToolBox::restore(const KConfigGroup &containmentGroup)
{
    if (!d->movable) {
        return;
    }

    KConfigGroup group;
    if (containmentGroup.isValid()) {
        group = containmentGroup;
    } else {
        group = d->containment->config();
    }

    group = KConfigGroup(&group, "ToolBox");

    if (!group.hasKey("corner")) {
        return;
    }

    d->userMoved = true;
    setCorner(Corner(group.readEntry("corner", int(corner()))));

    int offset = group.readEntry("offset", 0);
    switch (corner()) {
        case InternalToolBox::TopLeft:
            setPos(0, 0);
            break;
        case InternalToolBox::Top:
            setPos(offset, 0);
            break;
        case InternalToolBox::TopRight:
            setPos(d->containment->size().width() - boundingRect().width(), 0);
            break;
        case InternalToolBox::Right:
            setPos(d->containment->size().width() - boundingRect().width(), offset);
            break;
        case InternalToolBox::BottomRight:
            setPos(d->containment->size().width() - boundingRect().width(), d->containment->size().height() - boundingRect().height());
            break;
        case InternalToolBox::Bottom:
            setPos(offset, d->containment->size().height() - boundingRect().height());
            break;
        case InternalToolBox::BottomLeft:
            setPos(0, d->containment->size().height() - boundingRect().height());
            break;
        case InternalToolBox::Left:
            setPos(0, offset);
            break;
    }
    //kDebug() << "marked as user moved" << pos()
    //         << (d->containment->containmentType() == Containment::PanelContainment);
}

void InternalToolBox::reposition()
{
    if (d->userMoved) {
        //FIXME: adjust for situations like changing of the available space
        restore();
        return;
    }

    if (d->containment->containmentType() == Containment::PanelContainment ||
        d->containment->containmentType() == Containment::CustomPanelContainment) {
        QRectF rect = boundingRect();
        if (d->containment->formFactor() == Vertical) {
            setCorner(InternalToolBox::Bottom);
            setPos(d->containment->geometry().width() / 2 - rect.width() / 2,
                   d->containment->geometry().height() - rect.height());
        } else {
            //defaulting to Horizontal right now
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                setPos(d->containment->geometry().left(),
                       d->containment->geometry().height() / 2 - rect.height() / 2);
                setCorner(InternalToolBox::Left);
            } else {
                setPos(d->containment->geometry().width() - rect.width(),
                       d->containment->geometry().height() / 2 - rect.height() / 2);
                setCorner(InternalToolBox::Right);
            }
        }

        //kDebug() << "got ourselves a panel containment, moving to" << pos();
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
                    setPos(avail.topLeft() - QPoint(0, avail.top()));
                    setCorner(InternalToolBox::Left);
                } else if (avail.left() > screenGeom.left()) {
                    setPos(avail.topLeft() - QPoint(boundingRect().width(), 0));
                    setCorner(InternalToolBox::Top);
                } else {
                    setPos(avail.topLeft());
                    setCorner(InternalToolBox::TopLeft);
                }
            } else {
                if (avail.top() > screenGeom.top()) {
                    setPos(avail.topRight() - QPoint(boundingRect().width(), -avail.top()));
                    setCorner(InternalToolBox::Right);
                } else if (avail.right() < screenGeom.right()) {
                    setPos(avail.topRight() - QPoint(boundingRect().width(), 0));
                    setCorner(InternalToolBox::Top);
                } else {
                    setPos(avail.topRight() - QPoint(boundingRect().width(), 0));
                    setCorner(InternalToolBox::TopRight);
                }
            }
        } else {
            if (QApplication::layoutDirection() == Qt::RightToLeft) {
                setPos(d->containment->mapFromScene(QPointF(d->containment->geometry().topLeft())));
                setCorner(InternalToolBox::TopLeft);
            } else {
                setPos(d->containment->mapFromScene(QPointF(d->containment->geometry().topRight())));
                setCorner(InternalToolBox::TopRight);
            }
        }
    }
}

} // plasma namespace

#include "internaltoolbox_p.moc"

