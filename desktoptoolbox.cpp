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

#include "desktoptoolbox_p.h"

#include <QGraphicsSceneHoverEvent>
#include <QPainter>
#include <QRadialGradient>
#include <QGraphicsView>

#include <plasma/theme.h>
#include <KColorScheme>

#include <KDebug>

#include <plasma/applet.h>


namespace Plasma
{

class EmptyGraphicsItem : public QGraphicsItem
{
    public:
        EmptyGraphicsItem(QGraphicsItem *parent)
            : QGraphicsItem(parent)
        {
            setAcceptsHoverEvents(true);
        }

        QRectF boundingRect() const
        {
            return QRectF(QPointF(0, 0), m_rect.size());
        }

        QRectF rect() const
        {
            return m_rect;
        }

        void setRect(const QRectF &rect)
        {
            //kDebug() << "setting rect to" << rect;
            prepareGeometryChange();
            m_rect = rect;
            setPos(rect.topLeft());
        }

        void paint(QPainter * p, const QStyleOptionGraphicsItem*, QWidget*)
        {
            Q_UNUSED(p)
            //p->setPen(Qt::red);
            //p->drawRect(boundingRect());
        }

    private:
        QRectF m_rect;
};

// used with QGrahphicsItem::setData
static const int ToolName = 7001;

class DesktopToolBox::Private
{
public:
    Private()
      : icon("plasma"),
        toolBacker(0),
        animCircleId(0),
        animHighlightId(0),
        animCircleFrame(0),
        animHighlightFrame(0),
        hovering(0)
    {}

    KIcon icon;
    EmptyGraphicsItem *toolBacker;
    int animCircleId;
    int animHighlightId;
    qreal animCircleFrame;
    qreal animHighlightFrame;
    bool hovering : 1;
};

DesktopToolBox::DesktopToolBox(QGraphicsItem *parent)
    : ToolBox(parent),
      d(new Private)
{
    connect(Plasma::Animator::self(), SIGNAL(movementFinished(QGraphicsItem*)), this, SLOT(toolMoved(QGraphicsItem*)));

    setZValue(10000000);
    setFlag(ItemClipsToShape, true);
    setFlag(ItemClipsChildrenToShape, false);
    setFlag(ItemIgnoresTransformations, true);
}

DesktopToolBox::~DesktopToolBox()
{
    delete d;
}

QRectF DesktopToolBox::boundingRect() const
{
    return QRectF(0, 0, -size()*2, size()*2);
}

void DesktopToolBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->save();
    painter->translate(boundingRect().topLeft());

    QColor color1 = KColorScheme(QPalette::Active, KColorScheme::Window,
                               Plasma::Theme::defaultTheme()->colorScheme()).background().color();
    color1.setAlpha(64);

    QColor color2 = KColorScheme(QPalette::Active, KColorScheme::Window,
                               Plasma::Theme::defaultTheme()->colorScheme()).foreground().color();
    color2.setAlpha(64);

    QPainterPath p = shape();
    QRadialGradient gradient(boundingRect().topLeft(), size() + d->animCircleFrame);
    gradient.setFocalPoint(boundingRect().topLeft());
    gradient.setColorAt(0, color1);
    gradient.setColorAt(.87, color1);
    gradient.setColorAt(.97, color2);
    color2.setAlpha(0);
    gradient.setColorAt(1, color2);
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(gradient);
    painter->drawPath(p);
    painter->restore();

    const qreal progress = d->animHighlightFrame;

    if (progress <= 0.9) {
        d->icon.paint(painter, QRect(QPoint((int)boundingRect().left() - iconSize().width() + 2, 2), iconSize()), Qt::AlignCenter, QIcon::Disabled, QIcon::Off);
    }

    if (progress > 0.1) {
        painter->save();
        painter->setOpacity(progress);
        d->icon.paint(painter, QRect(QPoint((int)boundingRect().left() - iconSize().width() + 2, 2), iconSize()));
        painter->restore();
    }

    painter->restore();
}

QPainterPath DesktopToolBox::shape() const
{
    QPainterPath path;
    int toolSize = size() + (int)d->animCircleFrame;
    path.arcTo(QRectF(boundingRect().left() - toolSize, boundingRect().top() - toolSize, toolSize*2, toolSize*2), 180, 90);

    return path;
}

void DesktopToolBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (showing() || d->hovering) {
        QGraphicsItem::hoverEnterEvent(event);
        return;
    }
    Plasma::Animator* animdriver = Plasma::Animator::self();
    if (d->animHighlightId) {
        animdriver->stopCustomAnimation(d->animHighlightId);
    }
    d->hovering = true;
    d->animHighlightId = animdriver->customAnimation(10, 240, Plasma::Animator::EaseInCurve, this, "animateHighlight");

    QGraphicsItem::hoverEnterEvent(event);
}

void DesktopToolBox::showToolBox()
{
    if (showing()) {
        return;
    }

    int maxwidth = 0;
    foreach (QGraphicsItem* tool, QGraphicsItem::children()) {
        if (!tool->isEnabled()) {
            continue;
        }
        maxwidth = qMax(static_cast<int>(tool->boundingRect().width()), maxwidth);
    }

    // put tools 5px from icon edge
    const int iconWidth = 32;
    int x = (int)boundingRect().left() - maxwidth - iconWidth - 5;
    int y = (int)boundingRect().top() + 5;
    Plasma::Animator* animdriver = Plasma::Animator::self();
    foreach (QGraphicsItem* tool, QGraphicsItem::children()) {
        if (tool == d->toolBacker) {
            continue;
        }

        if (!tool->isEnabled()) {
            if (tool->isVisible()) {
                const int height = static_cast<int>(tool->boundingRect().height());
                animdriver->moveItem(tool, Plasma::Animator::SlideOutMovement, QPoint(size() * 2, -height));
            }
            continue;
        }

        //kDebug() << "let's show and move" << tool << tool->boundingRect();
        tool->show();
        animdriver->moveItem(tool, Plasma::Animator::SlideInMovement, QPoint(x, y));
        //x += 0;
        y += static_cast<int>(tool->boundingRect().height()) + 5;
    }

    if (!d->toolBacker) {
        d->toolBacker = new EmptyGraphicsItem(this);
    }
    d->toolBacker->setRect(QRectF(QPointF(x, 0), QSizeF(maxwidth, y - 10)));
    d->toolBacker->show();

    if (d->animCircleId) {
        animdriver->stopCustomAnimation(d->animCircleId);
    }

    setShowing(true);
    // TODO: 10 and 200 shouldn't be hardcoded here. There needs to be a way to
    // match whatever the time is that moveItem() takes. Same in hoverLeaveEvent().
    d->animCircleId = animdriver->customAnimation(10, 240, Plasma::Animator::EaseInCurve, this, "animateCircle");
}

void DesktopToolBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //kDebug() << event->pos() << event->scenePos() << d->toolBacker->rect().contains(event->scenePos().toPoint());
    if (! d->hovering) {
        QGraphicsItem::hoverLeaveEvent(event);
        return;
    }

    hideToolBox();
    Plasma::Animator* animdriver = Plasma::Animator::self();
    if (d->animHighlightId) {
        animdriver->stopCustomAnimation(d->animHighlightId);
    }
    d->hovering = false;
    d->animHighlightId = animdriver->customAnimation(10, 240, Plasma::Animator::EaseOutCurve, this, "animateHighlight");

    QGraphicsItem::hoverLeaveEvent(event);
}

void DesktopToolBox::hideToolBox()
{
    if (!showing()) {
        return;
    }

    int x = size() * 2;
    int y = 0;
    Plasma::Animator* animdriver = Plasma::Animator::self();
    foreach (QGraphicsItem* tool, QGraphicsItem::children()) {
        if (tool == d->toolBacker) {
            continue;
        }

        const int height = static_cast<int>(tool->boundingRect().height());
        animdriver->moveItem(tool, Plasma::Animator::SlideOutMovement, QPoint(x, y-height));
    }

    if (d->animCircleId) {
        animdriver->stopCustomAnimation(d->animCircleId);
    }

    setShowing(false);
    d->animCircleId = animdriver->customAnimation(10, 240, Plasma::Animator::EaseOutCurve, this, "animateCircle");

    if (d->toolBacker) {
        d->toolBacker->hide();
    }
}

void DesktopToolBox::animateCircle(qreal progress)
{
    if (showing()) {
        d->animCircleFrame = size() * progress;
    } else {
        d->animCircleFrame = size() * (1.0 - progress);
    }

    if (progress >= 1) {
        d->animCircleId = 0;
    }

    update();
}

void DesktopToolBox::animateHighlight(qreal progress)
{
    if (d->hovering) {
        d->animHighlightFrame = progress;
    } else {
        d->animHighlightFrame = 1.0 - progress;
    }

    if (progress >= 1) {
        d->animHighlightId = 0;
    }

    update();
}

void DesktopToolBox::toolMoved(QGraphicsItem *item)
{
    //kDebug() << "geometry is now " << static_cast<Plasma::Widget*>(item)->geometry();
    if (!showing() &&
        QGraphicsItem::children().indexOf(static_cast<Plasma::Applet*>(item)) != -1) {
        item->hide();
    }
}

void DesktopToolBox::toggle()
{
    if (showing()) {
        hideToolBox();
    } else {
        showToolBox();
    }
}

} // plasma namespace

#include "desktoptoolbox_p.moc"

