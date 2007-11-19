/*
 *   Copyright 2007 by Kevin Ottens <ervin@kde.org>
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

#include "applethandle_p.h"

#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>

#include <KColorScheme>
#include <KIcon>

#include <cmath>

#include "applet.h"
#include "containment.h"
#include "theme.h"

namespace Plasma
{

AppletHandle::AppletHandle(Containment *parent, Applet *applet)
    : QObject(),
      QGraphicsItem(parent),
      m_buttonsOnRight(false),
      m_pressedButton(NoButton),
      m_containment(parent),
      m_applet(applet),
      m_opacity(0.0),
      m_anim(FadeIn),
      m_animId(0),
      m_angle(0.0),
      m_scale(1.0)
{
    KColorScheme colors(QPalette::Active, KColorScheme::View, Theme::self()->colors());
    m_gradientColor = colors.background(KColorScheme::NormalBackground).color();

    m_originalMatrix = m_applet->transform();
    m_rect = m_applet->boundingRect();
    m_rect = m_applet->mapToParent(m_rect).boundingRect();

    const int requiredHeight = (HANDLE_WIDTH * 2) + m_applet->hasConfigurationInterface() ? + (ICON_SIZE * 4) : (ICON_SIZE * 3);
    if (m_rect.height() < requiredHeight) {
        float delta = requiredHeight - m_rect.height();
        delta = delta/2.0;
        if (delta > 0.0) {
            m_rect.adjust(0.0, -delta, 0.0, delta);
        }
    }

    m_rect.adjust(-HANDLE_WIDTH, -HANDLE_WIDTH, HANDLE_WIDTH, HANDLE_WIDTH);

    if (m_applet->pos().x() <= ((HANDLE_WIDTH * 2) + ICON_SIZE)) {
        m_rect.adjust(0.0, 0.0, ICON_SIZE, 0.0);
        m_buttonsOnRight = true;
    } else {
        m_rect.adjust(- ICON_SIZE, 0.0, 0.0, 0.0);
    }

    m_applet->setParentItem(this);
    setAcceptsHoverEvents(true);
    startFading(FadeIn);
}

AppletHandle::~AppletHandle()
{
    QPointF center = m_applet->boundingRect().center();

    // TODO: Keep the rotation only, to apply the scaling force
    // a new pixel size to applets (avoid upscaling aliasing, and
    // insane downscaling), probably requires support in the Applet class.
    QTransform matrix = m_originalMatrix;
    matrix.translate(center.x(), center.y());
    matrix.rotateRadians(m_angle);
    matrix.scale(m_scale, m_scale);
    matrix.translate(-center.x(), -center.y());

    QPointF newPos = transform().inverted().map(m_applet->pos());

    m_applet->setParentItem(m_containment);
    m_applet->setPos(mapToParent(newPos));
    m_applet->setTransform(matrix);
}

Applet *AppletHandle::applet() const
{
    return m_applet;
}

QRectF Plasma::AppletHandle::boundingRect() const
{
    return m_rect;
}

void AppletHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->save();
    painter->setOpacity(m_opacity);

    painter->save();
    painter->setOpacity(m_opacity * 0.4);
    painter->setPen(Qt::NoPen);
    painter->setRenderHints(QPainter::Antialiasing);
    QLinearGradient gr(boundingRect().topLeft(), boundingRect().bottomRight());
    gr.setColorAt(0, m_gradientColor);
    gr.setColorAt(0.1, KColorScheme::shade(m_gradientColor, KColorScheme::LightShade));
    gr.setColorAt(1, KColorScheme::shade(m_gradientColor, KColorScheme::DarkShade));
    painter->setBrush(gr);
    painter->drawPath(Plasma::roundedRectangle(boundingRect(), 10));
    painter->restore();

    QPointF point = m_rect.topLeft();

    if (m_buttonsOnRight) {
        point += QPointF(m_rect.width() - ICON_SIZE - HANDLE_WIDTH, HANDLE_WIDTH);
    } else {
        point+= QPointF(HANDLE_WIDTH / 2, HANDLE_WIDTH);
    }

    QPointF shiftC;
    QPointF shiftD;
    QPointF shiftR;

    switch(m_pressedButton)
    {
    case ConfigureButton:
        shiftC = QPointF(2, 2);
        break;
    case RemoveButton:
        shiftD = QPointF(2, 2);
        break;
    case RotateButton:
        shiftR = QPointF(2, 2);
        break;
    default:
        break;
    }

    if (m_applet->hasConfigurationInterface()) {
        painter->drawPixmap(point + shiftC, KIcon("configure").pixmap(ICON_SIZE, ICON_SIZE));
    }

    point += QPointF(0.0, ICON_SIZE + ICON_MARGIN);
    painter->drawPixmap(point + shiftR, KIcon("transform-rotate").pixmap(ICON_SIZE, ICON_SIZE));

    point += QPointF(0.0, ICON_SIZE * 2 + ICON_MARGIN);
    painter->drawPixmap(point + shiftD, KIcon("edit-delete").pixmap(ICON_SIZE, ICON_SIZE));

    painter->restore();
}

AppletHandle::ButtonType AppletHandle::mapToButton(const QPointF &point) const
{
    QPointF basePoint = m_rect.topLeft();

    if (m_buttonsOnRight) {
        basePoint+= QPointF(m_rect.width() - ICON_SIZE, HANDLE_WIDTH);
    } else {
        basePoint+= QPointF(HANDLE_WIDTH, HANDLE_WIDTH);
    }

    QPolygonF activeArea = QPolygonF(QRectF(basePoint, QSizeF(ICON_SIZE, ICON_SIZE)));

    if (m_applet->hasConfigurationInterface()) {
        if (activeArea.containsPoint(point, Qt::OddEvenFill)) {
            return ConfigureButton;
        }
        activeArea.translate(QPointF(0.0, ICON_SIZE + ICON_MARGIN));
    }

    if (activeArea.containsPoint(point, Qt::OddEvenFill)) {
        return RotateButton;
    }

    activeArea.translate(QPointF(0.0, ICON_SIZE * 2 + ICON_MARGIN));
    if (activeArea.containsPoint(point, Qt::OddEvenFill)) {
        return RemoveButton;
    }

    return MoveButton;
    //return m_applet->mapToParent(m_applet->shape()).contains(point) ? NoButton : MoveButton;
}

void AppletHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_pressedButton = mapToButton(event->pos());
        event->accept();
        update();
        return;
    }

    QGraphicsItem::mousePressEvent(event);
}

void AppletHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    ButtonType releasedAtButton = mapToButton(event->pos());

    if (event->button()==Qt::LeftButton && m_pressedButton==releasedAtButton) {
        if (m_pressedButton==ConfigureButton) {
            //FIXME: Remove this call once the configuration management change was done
            m_containment->emitLaunchActivated();
            m_applet->showConfigurationInterface();
        } else if (m_pressedButton==RemoveButton) {
            Phase::self()->animateItem(m_applet, Phase::Disappear);
            forceDisappear();
        }
    }

    m_pressedButton = NoButton;
    update();
}

qreal _k_distanceForPoint(QPointF point)
{
    return ::sqrt(point.x()*point.x()+point.y()*point.y());
}

qreal _k_angleForPoints(const QPointF &center, const QPointF &pt1, const QPointF &pt2)
{
    QPointF vec1 = pt1 - center;
    QPointF vec2 = pt2 - center;

    qreal alpha = ::atan2(vec1.y(), vec1.x());
    qreal beta = ::atan2(vec2.y(), vec2.x());

    return beta - alpha;
}

void AppletHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    static const qreal snapAngle = 3.14159 / 2.0;

    if (m_pressedButton == MoveButton) {
        QPointF delta = event->pos()-event->lastPos();
        setPos(pos()+delta);
    } else if (m_pressedButton == RotateButton) {
        if (_k_distanceForPoint(event->pos()-event->lastPos()) <= 1.0) {
            return;
        }

        QPointF pressPos = mapFromScene(event->buttonDownScenePos(Qt::LeftButton));

        QRectF rect = QRectF(m_applet->pos(), m_applet->size());
        QPointF center = rect.center();

        m_angle = _k_angleForPoints(center, pressPos, event->pos());

        if (fabs(remainder(m_angle, snapAngle)) < 0.15) {
            m_angle = m_angle - remainder(m_angle, snapAngle);
        }

        m_scale = _k_distanceForPoint(event->pos()-center) / _k_distanceForPoint(pressPos-center);

        QTransform matrix;
        matrix.translate(center.x(), center.y());
        matrix.rotateRadians(m_angle);
        matrix.scale(m_scale, m_scale);
        matrix.translate(-center.x(), -center.y());
        setTransform(matrix);
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

void AppletHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    startFading(FadeIn);
}

void AppletHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    startFading(FadeOut);
}

void AppletHandle::fadeAnimation(qreal progress)
{
    qreal endOpacity = 1.0;
    if (m_anim==FadeOut) {
        endOpacity = 0.0;
    }

    m_opacity += (endOpacity-m_opacity)*progress;

    if (progress>=1.0 && m_anim==FadeOut) {
        emit disappearDone(this);
    }

    update();
}

void AppletHandle::startFading(FadeType anim)
{
    if (m_animId!=0) {
        Phase::self()->stopCustomAnimation(m_animId);
    }

    qreal time = 250;

    if (anim==FadeIn) {
        time *= 1.0-m_opacity;
    } else {
        time *= m_opacity;
    }

    m_anim = anim;
    m_animId = Phase::self()->customAnimation(40, (int)time,
                                              Phase::EaseInOutCurve,
                                              this, "fadeAnimation");
}

void AppletHandle::forceDisappear()
{
    setAcceptsHoverEvents(false);
    startFading(FadeOut);
}

}

#include "applethandle_p.moc"
