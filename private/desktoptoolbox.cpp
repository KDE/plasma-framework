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
#include <QAction>

#include <kcolorscheme.h>
#include <kdebug.h>

#include <plasma/theme.h>
#include <plasma/paintutils.h>
#include <plasma/framesvg.h>

#include <plasma/applet.h>
#include <plasma/containment.h>
#include <plasma/widgets/iconwidget.h>

namespace Plasma
{

class EmptyGraphicsItem : public QGraphicsItem
{
    public:
        EmptyGraphicsItem(QGraphicsItem *parent)
            : QGraphicsItem(parent)
        {
            setAcceptsHoverEvents(true);
            m_background = new Plasma::FrameSvg();
            m_background->setImagePath("widgets/translucentbackground");
        }

        ~EmptyGraphicsItem()
        {
            delete m_background;
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
            qreal left, top, right, bottom;
            m_background->getMargins(left, top, right, bottom);

            prepareGeometryChange();
            m_rect = rect.adjusted(-left, -top, right, bottom);
            setPos(m_rect.topLeft());
            m_background->resizeFrame(m_rect.size());
        }

        void paint(QPainter *p, const QStyleOptionGraphicsItem *, QWidget *)
        {
            m_background->paintFrame(p);
        }

    private:
        QRectF m_rect;
        Plasma::FrameSvg *m_background;
};

// used with QGrahphicsItem::setData
static const int ToolName = 7001;

class DesktopToolBoxPrivate
{
public:
    DesktopToolBoxPrivate(DesktopToolBox *toolbox)
      : q(toolbox),
        containment(0),
        icon("plasma"),
        toolBacker(0),
        animCircleId(0),
        animHighlightId(0),
        animCircleFrame(0),
        animHighlightFrame(0),
        hovering(0)
    {}

    bool needsToolBarBehaviour()
    {
        bool changed = false;

        if (containment && viewTransform != q->viewTransform()) {
            viewTransform = q->viewTransform();
            changed = true;
        }

        if (changed || q->isToolbar() != viewTransform.isScaling()) {
            if (!q->isToolbar()) {
                q->setIsToolbar(true);
                q->showToolBox();
            } else {
                q->setIsToolbar(false);
                q->hideToolBox();
            }
        }

        return q->isToolbar();
    }

    DesktopToolBox *q;
    Containment *containment;
    KIcon icon;
    EmptyGraphicsItem *toolBacker;
    int animCircleId;
    int animHighlightId;
    qreal animCircleFrame;
    qreal animHighlightFrame;
    QRect shapeRect;
    QColor fgColor;
    QColor bgColor;
    QTransform viewTransform;
    bool hovering : 1;
};

DesktopToolBox::DesktopToolBox(Containment *parent)
    : ToolBox(parent),
      d(new DesktopToolBoxPrivate(this))
{
    d->containment = parent;
    setZValue(10000000);
    setFlag(ItemClipsToShape, true);
    setFlag(ItemClipsChildrenToShape, false);
    setFlag(ItemIgnoresTransformations, true);
    setIsMovable(true);
    assignColors();

    connect(Plasma::Animator::self(), SIGNAL(movementFinished(QGraphicsItem*)),
            this, SLOT(toolMoved(QGraphicsItem*)));
    connect(this, SIGNAL(toggled()), this, SLOT(toggle()));
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()),
            this, SLOT(assignColors()));
}

DesktopToolBox::~DesktopToolBox()
{
    delete d;
}

QRectF DesktopToolBox::boundingRect() const
{
    return QRectF(0, 0, size(), size());
}

void DesktopToolBox::assignColors()
{
    d->bgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor);
    d->fgColor = Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor);
}

void DesktopToolBox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (d->needsToolBarBehaviour()) {
        return;
    }

    QPainterPath p = shape();

    QPoint iconPos;
    QPointF gradientCenter;
    const QRectF rect = boundingRect();
    const QSize icons = iconSize();

    switch (corner()) {
    case TopRight:
        iconPos = QPoint((int)rect.right() - icons.width() + 2, 2);
        gradientCenter = rect.topRight();
        break;
    case Top:
        iconPos = QPoint(rect.center().x() - icons.width() / 2, 2);
        gradientCenter = QPoint(rect.center().x(), rect.y());
        break;
    case TopLeft:
        iconPos = QPoint(2, 2);
        gradientCenter = rect.topLeft();
        break;
    case Left:
        iconPos = QPoint(2, rect.center().y() - icons.height() / 2);
        gradientCenter = QPointF(rect.left(), rect.center().y());
        break;
    case Right:
        iconPos = QPoint((int)rect.right() - icons.width() + 2,
                         rect.center().y() - icons.height() / 2);
        gradientCenter = QPointF(rect.right(), rect.center().y());
        break;
    case BottomLeft:
        iconPos = QPoint(2, rect.bottom() - icons.height() - 2);
        gradientCenter = rect.bottomLeft();
        break;
    case Bottom:
        iconPos = QPoint(rect.center().x() - icons.width() / 2,
                         rect.bottom() - icons.height() - 2);
        gradientCenter = QPointF(rect.center().x(), rect.bottom());
        break;
    case BottomRight:
    default:
        iconPos = QPoint((int)rect.right() - icons.width() + 2,
                         (int)rect.bottom() - icons.height() - 2);
        gradientCenter = rect.bottomRight();
        break;
    }

    d->bgColor.setAlpha(64);
    d->fgColor.setAlpha(64);
    QRadialGradient gradient(gradientCenter, size() + d->animCircleFrame);
    gradient.setFocalPoint(gradientCenter);
    gradient.setColorAt(0, d->bgColor);
    gradient.setColorAt(.87, d->bgColor);
    gradient.setColorAt(.97, d->fgColor);
    d->fgColor.setAlpha(0);
    gradient.setColorAt(1, d->fgColor);
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(gradient);
    painter->drawPath(p);
    painter->restore();

    const qreal progress = d->animHighlightFrame;

    if (qFuzzyCompare(qreal(1.0), progress)) {
        d->icon.paint(painter, QRect(iconPos, iconSize()));
    } else if (qFuzzyCompare(qreal(1.0), 1 + progress)) {
        d->icon.paint(painter, QRect(iconPos, iconSize()),
                      Qt::AlignCenter, QIcon::Disabled, QIcon::Off);
    } else {
        QPixmap disabled = d->icon.pixmap(iconSize(), QIcon::Disabled, QIcon::Off);
        QPixmap enabled = d->icon.pixmap(iconSize());
        QPixmap result = PaintUtils::transition(
            d->icon.pixmap(iconSize(), QIcon::Disabled, QIcon::Off),
            d->icon.pixmap(iconSize()), progress);
        painter->drawPixmap(QRect(iconPos, iconSize()), result);
    }
}

QPainterPath DesktopToolBox::shape() const
{
    const QRectF rect = boundingRect();
    const int w = rect.width();
    const int h = rect.height();

    QPainterPath path;
    switch (corner()) {
    case BottomLeft:
        path.moveTo(rect.bottomLeft());
        path.arcTo(QRectF(rect.left() - w, rect.top(), w * 2, h * 2), 0, 90);
        break;
    case BottomRight:
        path.moveTo(rect.bottomRight());
        path.arcTo(QRectF(rect.left(), rect.top(), w * 2, h * 2), 90, 90);
        break;
    case TopRight:
        path.moveTo(rect.topRight());
        path.arcTo(QRectF(rect.left(), rect.top() - h, w * 2, h * 2), 180, 90);
        break;
    case TopLeft:
        path.arcTo(QRectF(rect.left() - w, rect.top() - h, w * 2, h * 2), 270, 90);
        break;
    default:
        path.addRect(rect);
        break;
    }

    return path;
}

void DesktopToolBox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    if (showing() || d->hovering) {
        QGraphicsItem::hoverEnterEvent(event);
        return;
    }
    Plasma::Animator *animdriver = Plasma::Animator::self();
    if (d->animHighlightId) {
        animdriver->stopCustomAnimation(d->animHighlightId);
    }
    d->hovering = true;
    d->animHighlightId =
        animdriver->customAnimation(
            10, 240, Plasma::Animator::EaseInCurve, this, "animateHighlight");

    QGraphicsItem::hoverEnterEvent(event);
}

void DesktopToolBox::showToolBox()
{
    if (showing() && !isToolbar()) {
        return;
    }

    // put tools 5px from icon edge
    const int iconWidth = 32;
    int x;
    int y;
    switch (corner()) {
    case TopRight:
        x = (int)boundingRect().right() - iconWidth - 5;
        y = (int)boundingRect().top() + 10;
        break;
    case Top:
        x = (int)boundingRect().center().x() - iconWidth;
        y = (int)boundingRect().top() + iconWidth + 10;
        break;
    case TopLeft:
        x = (int)boundingRect().left() + iconWidth + 5;
        y = (int)boundingRect().top() + 10;
        break;
    case Left:
        x = (int)boundingRect().left() + iconWidth + 5;
        y = (int)boundingRect().center().y() - iconWidth;
        break;
    case Right:
        x = (int)boundingRect().right() - iconWidth - 5;
        y = (int)boundingRect().center().y() - iconWidth;
        break;
    case BottomLeft:
        x = (int)boundingRect().left() + iconWidth + 5;
        y = (int)boundingRect().bottom() - 5;
        break;
    case Bottom:
        x = (int)boundingRect().center().x() - iconWidth;
        y = (int)boundingRect().bottom() - iconWidth - 5;
        break;
    case BottomRight:
    default:
        x = (int)boundingRect().right() - iconWidth - 5;
        y = (int)boundingRect().bottom() - iconWidth - 5;
        break;
    }

    int startY = y;

    // find our theoretical X and Y end coordinates

    int maxWidth = 0;
    int maxHeight = 0;
    int totalWidth = 0;

    foreach (QGraphicsItem *tool, QGraphicsItem::children()) {
        if (tool == d->toolBacker) {
            continue;
        }

        Plasma::IconWidget *icon = qgraphicsitem_cast<Plasma::IconWidget *>(tool);
        if (icon) {
            if (d->viewTransform.isScaling() && d->viewTransform.m11() < Plasma::scalingFactor(Plasma::GroupZoom)) {
                icon->setText(QString());
                icon->resize(icon->sizeFromIconSize(22));
            } else {
                icon->setText(icon->action()->text());
                icon->resize(icon->sizeFromIconSize(22));
            }
        }

        if (tool->isEnabled()) {
            //kDebug() << tool << "is enabled";
            y += 5;
            QSize toolSize = tool->boundingRect().size().toSize();
            totalWidth += toolSize.width() + 5;

            maxWidth = qMax(toolSize.width(), maxWidth);
            maxHeight = qMax(toolSize.height(), maxHeight);
            y += static_cast<int>(tool->boundingRect().height());
        }
    }

    if (corner() == TopRight || corner() == Right || corner() == BottomRight) {
        x -= maxWidth;
    }

    y += 5;
    // the rect the tools back should have
    QRectF backerRect = QRectF(QPointF(x, startY), QSizeF(maxWidth + 10, y - startY));

    if (isToolbar()) {
        QPointF topRight;

        //could that cast ever fail?
        if (d->containment) {
            topRight = d->viewTransform.map(mapFromParent(d->containment->boundingRect().topRight()));
        } else {
            topRight = boundingRect().topRight();
        }

        backerRect.setSize(QSize(totalWidth, maxHeight));
        backerRect.moveTopRight(topRight);
    }
    //kDebug() << "starting at" <<  x << startY;

    // now check that is actually fits within the parent's boundaries
    backerRect = mapToParent(backerRect).boundingRect();
    QSizeF parentSize = parentWidget()->size();
    if (backerRect.x() < 5) {
        backerRect.moveLeft(5);
    } else if (backerRect.right() > parentSize.width() - 5) {
        backerRect.moveRight(parentSize.width() - 5);
    }

    if (backerRect.y() < 5) {
        backerRect.moveTop(5);
    } else if (backerRect.bottom() > parentSize.height() - 5) {
        backerRect.moveBottom(parentSize.height() - 5);
    }

    // re-map our starting points back to our coordinate system
    backerRect = mapFromParent(backerRect).boundingRect();
    x = backerRect.x() + 5;
    y = backerRect.y();

    // now move the items
    Plasma::Animator *animdriver = Plasma::Animator::self();
    foreach (QGraphicsItem *tool, QGraphicsItem::children()) {
        if (tool == d->toolBacker) {
            continue;
        }

        if (tool->isEnabled()) {
            if (isToolbar()) {
                //kDebug() << tool << "is enabled";
                x += 5;
                //kDebug() << "let's show and move" << tool << tool->boundingRect();
                tool->show();
                animdriver->moveItem(tool, Plasma::Animator::SlideInMovement, QPoint(x, y));

                x += static_cast<int>(tool->boundingRect().width());
            } else {
                //kDebug() << tool << "is enabled";
                y += 5;
                //kDebug() << "let's show and move" << tool << tool->boundingRect();
                tool->show();
                animdriver->moveItem(tool, Plasma::Animator::SlideInMovement, QPoint(x, y));
                //x += 0;
                y += static_cast<int>(tool->boundingRect().height());
            }
        } else if (tool->isVisible()) {
            // disabled, but visible, so hide it!
            const int height = static_cast<int>(tool->boundingRect().height());
            animdriver->moveItem(tool, Plasma::Animator::SlideOutMovement,
                                 toolPosition(height));
        }
    }

    if (!d->toolBacker) {
        d->toolBacker = new EmptyGraphicsItem(this);
    }

    d->toolBacker->setRect(backerRect);
    d->toolBacker->show();

    if (d->animCircleId) {
        animdriver->stopCustomAnimation(d->animCircleId);
    }

    setShowing(true);
}

void DesktopToolBox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    //kDebug() << event->pos() << event->scenePos()
    //         << d->toolBacker->rect().contains(event->scenePos().toPoint());
    if (!d->hovering || isToolbar()) {
        QGraphicsItem::hoverLeaveEvent(event);
        return;
    }

    hideToolBox();
    Plasma::Animator *animdriver = Plasma::Animator::self();
    if (d->animHighlightId) {
        animdriver->stopCustomAnimation(d->animHighlightId);
    }
    d->hovering = false;
    d->animHighlightId =
        animdriver->customAnimation(
            10, 240, Plasma::Animator::EaseOutCurve, this, "animateHighlight");

    QGraphicsItem::hoverLeaveEvent(event);
}

void DesktopToolBox::hideToolBox()
{
    if (!showing()) {
        return;
    }

    Plasma::Animator *animdriver = Plasma::Animator::self();
    foreach (QGraphicsItem *tool, QGraphicsItem::children()) {
        if (tool == d->toolBacker) {
            continue;
        }

        const int height = static_cast<int>(tool->boundingRect().height());
        animdriver->moveItem(tool, Plasma::Animator::SlideOutMovement, toolPosition(height));
    }

    if (d->animCircleId) {
        animdriver->stopCustomAnimation(d->animCircleId);
    }

    setShowing(false);

    if (d->toolBacker) {
        d->toolBacker->hide();
    }
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
    if (isToolbar()) {
        return;
    }

    if (showing()) {
        hideToolBox();
    } else {
        showToolBox();
    }
}

} // plasma namespace

#include "desktoptoolbox_p.moc"
