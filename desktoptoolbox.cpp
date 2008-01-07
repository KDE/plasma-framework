/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
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

#include <plasma/theme.h>
#include <KColorScheme>

#include <KDebug>

#include "widgets/widget.h"

namespace Plasma
{

// used with QGrahphicsItem::setData
static const int ToolName = 7001;

DesktopToolbox::DesktopToolbox(QGraphicsItem *parent)
    : QGraphicsItem(parent),
      m_icon("plasma"),
      m_size(50),
      m_showing(false),
      m_animId(0),
      m_animFrame(0)
{
    setAcceptsHoverEvents(true);
    setZValue(10000);
    setFlag(ItemClipsToShape, true);
    setFlag(ItemClipsChildrenToShape, false);

    connect(Plasma::Phase::self(), SIGNAL(movementComplete(QGraphicsItem*)), this, SLOT(toolMoved(QGraphicsItem*)));
}

/*QRectF DesktopToolbox::sizeHint() const
{
    return boundingRect();
}*/

QRectF DesktopToolbox::boundingRect() const
{
    return QRectF(0, 0, m_size*2, m_size*2);
}

void DesktopToolbox::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    QColor color1 = KColorScheme(QPalette::Active, KColorScheme::Window,
                               Plasma::Theme::self()->colors()).background().color();
    color1.setAlpha(64);

    QColor color2 = KColorScheme(QPalette::Active, KColorScheme::Window,
                               Plasma::Theme::self()->colors()).foreground().color();
    color2.setAlpha(64);

    QPainterPath p = shape();
    QRadialGradient gradient(QPoint(m_size*2, 0), m_size + m_animFrame);
    gradient.setFocalPoint(QPointF(m_size*2, 0));
    gradient.setColorAt(0, color1);
    gradient.setColorAt(.87, color1);
    gradient.setColorAt(.97, color2);
    gradient.setColorAt(1, QColor(255, 255, 255, 0));
    painter->save();
    painter->setPen(Qt::NoPen);
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setBrush(gradient);
    painter->drawPath(p);
    painter->restore();
    m_icon.paint(painter, QRect(m_size*2 - 34, 2, 32, 32));
}

QPainterPath DesktopToolbox::shape() const
{
    QPainterPath path;
    int size = m_size + m_animFrame;
    path.moveTo(m_size*2, 0);
    path.arcTo(QRectF(m_size*2 - size, -size, size*2, size*2), 180, 90);
    return path;
}

void DesktopToolbox::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
//    Plasma::Phase::self()->moveItem(this, Phase::SlideIn, QPoint(-25, -25));
    int maxwidth = 0;
    foreach (QGraphicsItem* tool, QGraphicsItem::children()) {
        if (!tool->isEnabled()) {
            continue;
        }
        maxwidth = qMax(static_cast<int>(tool->boundingRect().width()),
                        maxwidth);
    }
    // put tools 5px from screen edge
    int x = m_size*2 - maxwidth - 5; // pos().x();
    int y = 5; // pos().y();
    Plasma::Phase* phase = Plasma::Phase::self();
    foreach (QGraphicsItem* tool, QGraphicsItem::children()) {
        if (!tool->isEnabled()) {
            continue;
        }

        //kDebug(1209) << "let's show and move" << tool << tool->boundingRect();
        tool->show();
        phase->moveItem(tool, Plasma::Phase::SlideIn, QPoint(x, y));
        //x += 0;
        y += static_cast<int>(tool->boundingRect().height()) + 5;
    }

    if (m_animId) {
        phase->stopCustomAnimation(m_animId);
    }

    m_showing = true;
    // TODO: 10 and 200 shouldn't be hardcoded here. There needs to be a way to
    // match whatever the time is that moveItem() takes. Same in hoverLeaveEvent().
    m_animId = phase->customAnimation(10, 250, Plasma::Phase::EaseInCurve, this, "animate");
    QGraphicsItem::hoverEnterEvent(event);
}

void DesktopToolbox::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
//    Plasma::Phase::self->moveItem(this, Phase::SlideOut, boundingRect()QPoint(-50, -50));
    int x = m_size*2;
    int y = 0;
    Plasma::Phase* phase = Plasma::Phase::self();
    foreach (QGraphicsItem* tool, QGraphicsItem::children()) {
         const int height = static_cast<int>(tool->boundingRect().height());
         phase->moveItem(tool, Plasma::Phase::SlideOut, QPoint(x, y-height)); // FIXME: make me faster (~150-200 ms)
    }

    if (m_animId) {
        phase->stopCustomAnimation(m_animId);
    }

    m_showing = false;
    m_animId = phase->customAnimation(10, 150, Plasma::Phase::EaseOutCurve, this, "animate");
    QGraphicsItem::hoverLeaveEvent(event);
}

void DesktopToolbox::animate(qreal progress)
{
    if (m_showing) {
        m_animFrame = static_cast<int>(m_size * progress);
    } else {
        m_animFrame = static_cast<int>(m_size * (1.0 - progress));
    }

    //kDebug(1209) << "animating at" << progress << "for" << m_animFrame;

    if (progress >= 1) {
        m_animId = 0;
    }

    update();
}

void DesktopToolbox::toolMoved(QGraphicsItem *item)
{
    //kDebug(1209) << "geometry is now " << static_cast<Plasma::Widget*>(item)->geometry();
    if (!m_showing &&
        QGraphicsItem::children().indexOf(static_cast<Plasma::Widget*>(item)) != -1) {
        item->hide();
    }
}

void DesktopToolbox::addTool(QGraphicsItem *tool, const QString &name)
{
    if (!tool) {
        return;
    }

    tool->hide();
    const int height = static_cast<int>(tool->boundingRect().height());
    tool->setPos(QPoint(m_size*2,-height));
    tool->setZValue(zValue() + 1);
    tool->setParentItem(this);
    tool->setData(ToolName, name);
}

void DesktopToolbox::enableTool(const QString &toolName, bool visible)
{
    //kDebug(1209) << (visible? "enabling" : "disabling") << "tool" << toolName;
    QGraphicsItem *t = tool(toolName);

    if (t) {
        t->setEnabled(visible);
    }
}

bool DesktopToolbox::isToolEnabled(const QString &toolName) const
{
    QGraphicsItem *t = tool(toolName);

    if (t) {
        return t->isEnabled();
    }

    return false;
}

QGraphicsItem* DesktopToolbox::tool(const QString &toolName) const
{
    foreach (QGraphicsItem *child, QGraphicsItem::children()) {
        //kDebug(1209) << "checking tool" << child << child->data(ToolName);
        if (child->data(ToolName).toString() == toolName) {
            //kDebug(1209) << "tool found!";
            return child;
        }
    }

    return 0;
}

} // plasma namespace

#include "desktoptoolbox_p.moc"

