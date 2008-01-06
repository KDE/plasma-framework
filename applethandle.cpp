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
#include <QtGui/QApplication>

#include <KColorScheme>
#include <KIcon>

#include <cmath>
#include <math.h>

#include "applet.h"
#include "containment.h"
#include "corona.h"
#include "applet.h"
#include "theme.h"

namespace Plasma
{

qreal _k_angleForPoints(const QPointF &center, const QPointF &pt1, const QPointF &pt2);

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
      m_scaleWidth(1.0),
      m_scaleHeight(1.0)
{
    KColorScheme colors(QPalette::Active, KColorScheme::View, Theme::self()->colors());
    m_gradientColor = colors.background(KColorScheme::NormalBackground).color();

    QTransform originalMatrix = m_applet->transform();
    QRectF rect(m_applet->boundingRect());
    QPointF center = rect.center();
    originalMatrix.translate(center.x(), center.y());

    qreal cosine = originalMatrix.m11();
    qreal sine = originalMatrix.m12();

    m_originalAngle = _k_angleForPoints(QPointF(0, 0),
                                        QPointF(1, 0),
                                        QPointF(cosine, sine));

    calculateSize();
    m_applet->setParentItem(this);
    connect(m_applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletDestroyed()));
    setAcceptsHoverEvents(true);
    startFading(FadeIn);
}

AppletHandle::~AppletHandle()
{
    if (m_applet) {
        QRectF rect(m_applet->boundingRect());
        QPointF center = rect.center();

        QPointF newPos = transform().inverted().map(m_applet->pos());
        m_applet->setPos(mapToParent(newPos));
        QTransform matrix;
        matrix.translate(center.x(), center.y());
        matrix.rotateRadians(m_originalAngle+m_angle);
        matrix.translate(-center.x(), -center.y());
        m_applet->setTransform(matrix);
        m_applet->setParentItem(m_containment);
    }
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
    QPointF shiftM;

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
    case ResizeButton:
        shiftM = QPointF(2, 2);
        break;
    default:
        break;
    }

    if (m_applet && m_applet->hasConfigurationInterface()) {
        painter->drawPixmap(point + shiftC, KIcon("configure").pixmap(ICON_SIZE, ICON_SIZE));
        point += QPointF(0.0, ICON_SIZE + ICON_MARGIN);
    }
    painter->drawPixmap(point + shiftR, KIcon("transform-rotate").pixmap(ICON_SIZE, ICON_SIZE));

    point += QPointF(0.0, ICON_SIZE + ICON_MARGIN);
    painter->drawPixmap(point + shiftM, KIcon("transform-move").pixmap(ICON_SIZE, ICON_SIZE)); // no transform-resize icon

    point += QPointF(0.0, ICON_SIZE + ICON_MARGIN * 2);
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

    if (m_applet && m_applet->hasConfigurationInterface()) {
        if (activeArea.containsPoint(point, Qt::OddEvenFill)) {
            return ConfigureButton;
        }
        activeArea.translate(QPointF(0.0, ICON_SIZE + ICON_MARGIN));
    }

    if (activeArea.containsPoint(point, Qt::OddEvenFill)) {
        return RotateButton;
    }

    activeArea.translate(QPointF(0.0, ICON_SIZE + ICON_MARGIN));
    if (activeArea.containsPoint(point, Qt::OddEvenFill)) {
        return ResizeButton;
    }

    activeArea.translate(QPointF(0.0, ICON_SIZE + ICON_MARGIN * 2));
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

    if (m_applet && event->button() == Qt::LeftButton) {
        switch (m_pressedButton) {
            case ResizeButton:
            case RotateButton: {
                if (m_scaleWidth > 0 && m_scaleHeight > 0) {
                    QRectF rect(m_applet->boundingRect());
                    const qreal newWidth = rect.width() * m_scaleWidth;
                    const qreal newHeight = rect.height() * m_scaleHeight;
                    m_applet->resetTransform();
                    m_applet->resize(newWidth, newHeight);
                    scale(1.0/m_scaleWidth, 1.0/m_scaleHeight);
                    moveBy((rect.width() - newWidth) / 2, (rect.height() - newHeight) / 2);
                    m_scaleWidth = m_scaleHeight = 0;
                }
                QRectF rect(boundingRect());
                QPointF center = rect.center();

                QTransform matrix;
                matrix.translate(center.x(), center.y());
                matrix.rotateRadians(m_originalAngle+m_angle);
                matrix.translate(-center.x(), -center.y());

                setTransform(matrix);
                m_applet->update();
                break;
            }
            case ConfigureButton:
                //FIXME: Remove this call once the configuration management change was done
                if (m_pressedButton == releasedAtButton) {
                    m_containment->emitLaunchActivated();
                    m_applet->showConfigurationInterface();
                }
                break;
            case RemoveButton:
                if (m_pressedButton == releasedAtButton) {
                    forceDisappear();
                    Phase::self()->animateItem(m_applet, Phase::Disappear);
                }
                break;
            default:
                break;
        }
    }

    m_pressedButton = NoButton;
    update();
}

qreal _k_distanceForPoint(QPointF point)
{
    return std::sqrt(point.x()*point.x()+point.y()*point.y());
}

qreal _k_angleForPoints(const QPointF &center, const QPointF &pt1, const QPointF &pt2)
{
    QPointF vec1 = pt1 - center;
    QPointF vec2 = pt2 - center;

    qreal alpha = std::atan2(vec1.y(), vec1.x());
    qreal beta = std::atan2(vec2.y(), vec2.x());

    return beta - alpha;
}

void AppletHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    static const qreal snapAngle = M_PI_2 /* $i 3.14159 / 2.0 */;

    if (!m_applet) {
        QGraphicsItem::mouseMoveEvent(event);
        return;
    }

    QPointF curPos = transform().map(event->pos());
    QPointF lastPos = transform().map(event->lastPos());
    QPointF delta = curPos-lastPos;

    if (m_pressedButton == MoveButton) {
        setPos(pos()+delta);
        // test for containment change
        if (!m_containment->sceneBoundingRect().contains(event->scenePos())) {
            // see which containment it belongs to
            Corona * corona = qobject_cast<Corona*>(scene());
            if (corona) {
                QList<Containment*> containments = corona->containments();
                for (int i = 0; i < containments.size(); ++i) {
                    if (containments[i]->sceneBoundingRect().contains(event->scenePos())) {
                        // add the applet to the new containment
                        // and take it from the old one
                        QPointF scenePosition = scenePos();
                        kDebug() << "moving to other containment with position" << pos() << event->scenePos();
                        kDebug() << "position before reparenting" << pos() << scenePos();
                        m_containment = containments[i];
                        //m_containment->addChild(m_applet);
                        //setParentItem(containments[i]);
                        m_containment->addApplet(m_applet);
                        setParentItem(m_containment);
                        m_applet->setParentItem(this);
                        setPos(m_containment->mapFromScene(scenePosition));
                        update();
                        break;
                    }
                }
            }
        }
    } else if (m_pressedButton == RotateButton ||
               m_pressedButton == ResizeButton) {
        if (_k_distanceForPoint(delta) <= 1.0) {
            return;
        }

        QPointF pressPos = mapFromScene(event->buttonDownScenePos(Qt::LeftButton));

        QRectF rect = QRectF(m_applet->pos(), m_applet->size());
        QPointF center = rect.center();

        if (m_pressedButton == RotateButton) {
            m_angle = _k_angleForPoints(center, pressPos, event->pos());

            if (fabs(remainder(m_originalAngle+m_angle, snapAngle)) < 0.15) {
                m_angle = m_angle - remainder(m_originalAngle+m_angle, snapAngle);
            }

            m_scaleWidth = m_scaleHeight = 1.0;
        } else {
            qreal w = m_applet->size().width();
            qreal h = m_applet->size().height();
            QSizeF min = m_applet->minimumSize();
            QSizeF max = m_applet->maximumSize();

            // If the applet doesn't have a minimum size, calculate based on a
            // minimum content area size of 16x16
            if (min.isEmpty()) {
                min = m_applet->boundingRect().size() - m_applet->contentRect().size();
                min += QSizeF(16, 16);
            }

            if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                // free resizing
                qreal newScaleWidth = 0;
                qreal newScaleHeight = 0;

                QPointF startDistance(pressPos - center);
                QPointF currentDistance(event->pos() - center);
                newScaleWidth = currentDistance.x() / startDistance.x();
                newScaleHeight = currentDistance.y() / startDistance.y();

                if (qAbs(newScaleWidth-1.0)<=0.1) {
                    newScaleWidth = 1.0;
                }
                if (qAbs(newScaleHeight-1.0)<=0.1) {
                    newScaleHeight = 1.0;
                }

                if (newScaleHeight * h < min.height()) {
                    m_scaleHeight = min.height() / h;
                } else if (newScaleHeight * h > max.height()) {
                    m_scaleHeight = max.height() / h;
                } else {
                    m_scaleHeight = newScaleHeight;
                }
                if (newScaleWidth * w < min.width()) {
                    m_scaleWidth = min.width() / w;
                } else if (newScaleWidth * w > max.width()) {
                    m_scaleWidth = max.width() / w;
                } else {
                    m_scaleWidth = newScaleWidth;
                }
            } else {
                // maintain aspect ratio
                qreal newScale = 0;

                newScale = _k_distanceForPoint(event->pos()-center) / _k_distanceForPoint(pressPos-center);
                if (qAbs(newScale-1.0)<=0.1) {
                    newScale = 1.0;
                }

                if (newScale * w < min.width() || newScale * h < min.height()) {
                    m_scaleWidth = m_scaleHeight = qMax(min.width() / w, min.height() / h);
                } else if (newScale * w > max.width() && newScale * h > max.height()) {
                    m_scaleWidth = m_scaleHeight = qMin(max.width() / w, max.height() / h);
                } else {
                    m_scaleHeight = m_scaleWidth = newScale;
                }
            }
        }

        QTransform matrix;
        matrix.translate(center.x(), center.y());
        matrix.rotateRadians(m_angle);
        matrix.scale(m_scaleWidth, m_scaleHeight);
        matrix.translate(-center.x(), -center.y());
        setTransform(matrix);
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

QVariant AppletHandle::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_applet) {
        m_applet->constraintsUpdated(Plasma::LocationConstraint);
    }
    return QGraphicsItem::itemChange(change, value);
}

void AppletHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    //kDebug() << "hover enter";
    startFading(FadeIn);
}

void AppletHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    //kDebug() << "hover leave";
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

void AppletHandle::appletDestroyed()
{
    m_applet = 0;
    deleteLater();
}

void AppletHandle::appletResized()
{
    prepareGeometryChange();
    calculateSize();
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
    m_animId = Phase::self()->customAnimation(40, (int)time, Phase::EaseInOutCurve, this, "fadeAnimation");
}

void AppletHandle::forceDisappear()
{
    setAcceptsHoverEvents(false);
    startFading(FadeOut);
}

void AppletHandle::calculateSize()
{
    m_rect = m_applet->boundingRect();
    m_rect = m_applet->mapToParent(m_rect).boundingRect();

    const int requiredHeight = (HANDLE_WIDTH * 2) + m_applet->hasConfigurationInterface() ?
                                                        ((ICON_SIZE + ICON_MARGIN) * 4) :
                                                        ((ICON_SIZE + ICON_MARGIN) * 3) +
                               ICON_MARGIN; // that last margin is blank space before the close button
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
}

}

#include "applethandle_p.moc"
