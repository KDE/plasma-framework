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
#include <KGlobalSettings>
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
      m_pressedButton(NoButton),
      m_containment(parent),
      m_applet(applet),
      m_opacity(0.0),
      m_anim(FadeIn),
      m_animId(0),
      m_angle(0.0),
      m_tempAngle(0.0),
      m_scaleWidth(1.0),
      m_scaleHeight(1.0),
      m_buttonsOnRight(false),
      m_pendingFade(false)
{
    KColorScheme colors(QPalette::Active, KColorScheme::View, Theme::self()->colors());
    m_gradientColor = colors.background(KColorScheme::NormalBackground).color();

    QTransform originalMatrix = m_applet->transform();
    QRectF rect(m_applet->boundingRect());
    QPointF center = rect.center();
    originalMatrix.translate(center.x(), center.y());

    qreal cosine = originalMatrix.m11();
    qreal sine = originalMatrix.m12();

    m_angle = _k_angleForPoints(QPointF(0, 0),
                                QPointF(1, 0),
                                QPointF(cosine, sine));

    m_applet->resetTransform();

    calculateSize();
    m_applet->setParentItem(this);

    rect = QRectF(m_applet->pos(), m_applet->size());
    center = rect.center();
    QTransform matrix;
    matrix.translate(center.x(), center.y());
    matrix.rotateRadians(m_angle);
    matrix.translate(-center.x(), -center.y());
    setTransform(matrix);
    m_hoverTimer = new QTimer(this);
    m_hoverTimer->setSingleShot(true);
    m_hoverTimer->setInterval(300);

    connect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(fadeIn()));
    connect(m_applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletDestroyed()));

    setAcceptsHoverEvents(true);
    m_hoverTimer->start();
}

AppletHandle::~AppletHandle()
{
    if (m_applet) {
        m_applet->removeSceneEventFilter(this);

        QRectF rect = QRectF(m_applet->pos(), m_applet->size());
        QPointF center = m_applet->mapFromParent(rect.center());

        QPointF newPos = transform().inverted().map(m_applet->pos());
        m_applet->setPos(mapToParent(newPos));

        QTransform matrix;
        matrix.translate(center.x(), center.y());
        matrix.rotateRadians(m_angle);
        matrix.translate(-center.x(), -center.y());
        m_applet->setTransform(matrix);

        m_applet->setParentItem(m_containment);

        m_applet->update(); // re-render the background, now we've transformed the applet
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
    QPainterPath path = Plasma::roundedRectangle(boundingRect(), 10);

    if (m_applet) {
        QPainterPath shape = m_applet->shape();

        if (!shape.isEmpty()) {
            path = path.subtracted(m_applet->mapToParent(m_applet->shape()));
        }
    }

    painter->drawPath(path);
    painter->restore();

    //XXX this code is duplicated in the next function
    QPointF basePoint = m_rect.topLeft() + QPointF(HANDLE_WIDTH / 2, HANDLE_WIDTH);
    QPointF step = QPointF(0, ICON_SIZE + ICON_MARGIN);
    QPointF separator = step + QPointF(0, ICON_MARGIN);

    if (m_buttonsOnRight) {
        basePoint += QPointF(m_rect.width() - ICON_SIZE - HANDLE_WIDTH, 0);
    }
    //end duplicate code

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

    painter->drawPixmap(basePoint + shiftM, KIcon("transform-move").pixmap(ICON_SIZE, ICON_SIZE)); //FIXME no transform-resize icon

    basePoint += step;
    painter->drawPixmap(basePoint + shiftR, KIcon("transform-rotate").pixmap(ICON_SIZE, ICON_SIZE));

    if (m_applet && m_applet->hasConfigurationInterface()) {
        basePoint += step;
        painter->drawPixmap(basePoint + shiftC, KIcon("configure").pixmap(ICON_SIZE, ICON_SIZE));
    }

    basePoint += separator;
    painter->drawPixmap(basePoint + shiftD, KIcon("edit-delete").pixmap(ICON_SIZE, ICON_SIZE));

    painter->restore();
}

AppletHandle::ButtonType AppletHandle::mapToButton(const QPointF &point) const
{
    //XXX this code is duplicated in the prev. function
    QPointF basePoint = m_rect.topLeft() + QPointF(HANDLE_WIDTH / 2, HANDLE_WIDTH);
    QPointF step = QPointF(0, ICON_SIZE + ICON_MARGIN);
    QPointF separator = step + QPointF(0, ICON_MARGIN);

    if (m_buttonsOnRight) {
        basePoint += QPointF(m_rect.width() - ICON_SIZE - HANDLE_WIDTH, 0);
    }
    //end duplicate code

    QPolygonF activeArea = QPolygonF(QRectF(basePoint, QSizeF(ICON_SIZE, ICON_SIZE)));

    if (activeArea.containsPoint(point, Qt::OddEvenFill)) {
        return ResizeButton;
    }

    activeArea.translate(step);
    if (activeArea.containsPoint(point, Qt::OddEvenFill)) {
        return RotateButton;
    }

    if (m_applet && m_applet->hasConfigurationInterface()) {
        activeArea.translate(step);
        if (activeArea.containsPoint(point, Qt::OddEvenFill)) {
            return ConfigureButton;
        }
    }

    activeArea.translate(separator);
    if (activeArea.containsPoint(point, Qt::OddEvenFill)) {
        return RemoveButton;
    }

    return MoveButton;
    //return m_applet->mapToParent(m_applet->shape()).contains(point) ? NoButton : MoveButton;
}

void AppletHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (m_pendingFade) {
        //m_pendingFade = false;
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_pressedButton = mapToButton(event->pos());
        //kDebug() << "button pressed:" << m_pressedButton;
        if (m_pressedButton != NoButton) {
            // when the mouse goes over a window, the applet is likely to
            // get a hover out event that we really don't want to respond to
            // so while we have a button pressed we intercept these events
            // and handle them ourselves here in AppletHandle
            m_applet->installSceneEventFilter(this);
        }
        event->accept();
        update();
        return;
    }

    QGraphicsItem::mousePressEvent(event);
}

void AppletHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //kDebug() << "button pressed:" << m_pressedButton << ", fade pending?" << m_pendingFade;
    if (m_applet) {
        m_applet->removeSceneEventFilter(this);
    }

    if (m_pendingFade) {
        startFading(FadeOut);
        m_pendingFade = false;
    }

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
                QRectF rect = QRectF(m_applet->pos(), m_applet->size());
                QPointF center = rect.center();

                m_angle += m_tempAngle;
                m_tempAngle = 0;

                QTransform matrix;
                matrix.translate(center.x(), center.y());
                matrix.rotateRadians(m_angle);
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
                        //kDebug() << "moving to other containment with position" << pos() << event->scenePos();
                        //kDebug() << "position before reparenting" << pos() << scenePos();
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
            m_tempAngle = _k_angleForPoints(center, pressPos, event->pos());

            if (fabs(remainder(m_angle+m_tempAngle, snapAngle)) < 0.15) {
                m_tempAngle = m_tempAngle - remainder(m_angle+m_tempAngle, snapAngle);
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

            bool ignoreAspectRatio = m_applet->aspectRatioMode() == Qt::IgnoreAspectRatio;

            if (QApplication::keyboardModifiers() & Qt::ControlModifier) {
                ignoreAspectRatio = !ignoreAspectRatio;
            }

            if (ignoreAspectRatio) {
                // free resizing
                qreal newScaleWidth = 0;
                qreal newScaleHeight = 0;

                QPointF startDistance(pressPos - center);
                QPointF currentDistance(event->pos() - center);
                newScaleWidth = currentDistance.x() / startDistance.x();
                newScaleHeight = currentDistance.y() / startDistance.y();

                if (qAbs(w - (newScaleWidth * w)) <= KGlobalSettings::dndEventDelay()) {
                    newScaleWidth = 1.0;
                }
                if (qAbs(h - (newScaleHeight * h)) <= KGlobalSettings::dndEventDelay()) {
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
                if (qAbs(h - (newScale * h)) <= KGlobalSettings::dndEventDelay()) {
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
        matrix.rotateRadians(m_angle+m_tempAngle);
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
    m_hoverTimer->start();
}

void AppletHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    //kDebug() << "hover leave" << m_pressedButton;
    m_hoverTimer->stop();

    if (m_pressedButton != NoButton) {
        m_pendingFade = true;
    } else {
        startFading(FadeOut);
    }
}

bool AppletHandle::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched == m_applet && event->type() == QEvent::GraphicsSceneHoverLeave) {
        hoverLeaveEvent(static_cast<QGraphicsSceneHoverEvent*>(event));
        return true;
    }

    return false;
}

void AppletHandle::fadeAnimation(qreal progress)
{
    qreal endOpacity = 1.0;
    if (m_anim==FadeOut) {
        endOpacity = 0.0;
    }

    m_opacity += (endOpacity-m_opacity)*progress;

    if (progress>=1.0 && m_anim==FadeOut) {
        if (m_applet) {
            m_applet->removeSceneEventFilter(this);
        }

        emit disappearDone(this);
    }

    update();
}

void AppletHandle::fadeIn()
{
    startFading(FadeIn);
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
    if (m_animId != 0) {
        Phase::self()->stopCustomAnimation(m_animId);
    }

    qreal time = 250;
    m_hoverTimer->stop();

    if (m_applet) {
        m_applet->removeSceneEventFilter(this);
    }

    if (anim == FadeIn) {
        time *= 1.0-m_opacity;
    } else {
        m_hoverTimer->stop();
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

    //XXX remember to update this if the number of buttons changes
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
