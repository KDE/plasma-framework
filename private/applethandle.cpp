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

#include "private/applethandle_p.h"

#include <QApplication>
#include <QBitmap>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QLinearGradient>
#include <QtGui/QPainter>
#include <QtGui/QApplication>
#include <QtGui/QMenu>

#include <kcolorscheme.h>
#include <kglobalsettings.h>
#include <kicon.h>
#include <kiconloader.h>
#include <kwindowsystem.h>

#include <cmath>
#include <math.h>

#include "applet.h"
#include "applet_p.h"
#include "containment.h"
#include "corona.h"
#include "paintutils.h"
#include "theme.h"
#include "view.h"
#include "framesvg.h"

namespace Plasma
{

qreal _k_distanceForPoint(QPointF point);
qreal _k_pointAngle(QPointF in);
QPointF _k_rotatePoint(QPointF in, qreal rotateAngle);

AppletHandle::AppletHandle(Containment *parent, Applet *applet, const QPointF &hoverPos)
    : QObject(),
      QGraphicsItem(parent),
      m_pressedButton(NoButton),
      m_containment(parent),
      m_applet(applet),
      m_iconSize(16),
      m_opacity(0.0),
      m_anim(FadeIn),
      m_animId(0),
      m_angle(0.0),
      m_backgroundBuffer(0),
      m_currentView(applet->view()),
      m_entryPos(hoverPos),
      m_buttonsOnRight(false),
      m_pendingFade(false)
{
    KColorScheme colorScheme(QPalette::Active, KColorScheme::View,
                             Theme::defaultTheme()->colorScheme());
    m_gradientColor = colorScheme.background(KColorScheme::NormalBackground).color();

    QTransform originalMatrix = m_applet->transform();
    m_applet->resetTransform();

    QRectF rect(m_applet->contentsRect());
    QPointF center = rect.center();
    originalMatrix.translate(center.x(), center.y());

    qreal cosine = originalMatrix.m11();
    qreal sine = originalMatrix.m12();

    m_angle = _k_pointAngle(QPointF(cosine, sine));

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
    m_hoverTimer->setInterval(333);

    m_leaveTimer = new QTimer(this);
    m_leaveTimer->setSingleShot(true);
    m_leaveTimer->setInterval(500);

    connect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverTimeout()));
    connect(m_leaveTimer, SIGNAL(timeout()), this, SLOT(leaveTimeout()));
    connect(m_applet, SIGNAL(destroyed(QObject*)), this, SLOT(appletDestroyed()));

    setAcceptsHoverEvents(true);
    m_hoverTimer->start();

    //icons
    m_configureIcons = new Svg(this);
    m_configureIcons->setImagePath("widgets/configuration-icons");
    //FIXME: this should be of course true, but works only if false
    m_configureIcons->setContainsMultipleImages(true);

    m_background = new FrameSvg(this);
    m_background->setImagePath("widgets/background");

    //We got to be able to see the applet while dragging to to another containment,
    //so we want a high zValue.
    //FIXME: apparently this doesn't work: sometimes an applet still get's drawn behind
    //the containment it's being dragged to, sometimes it doesn't.
    m_zValue = m_applet->zValue() - 1;
    m_applet->raise();
    m_applet->installSceneEventFilter(this);
    setZValue(m_applet->zValue());
}

AppletHandle::~AppletHandle()
{
    detachApplet();
    delete m_backgroundBuffer;
}

Applet *AppletHandle::applet() const
{
    return m_applet;
}

void AppletHandle::detachApplet ()
{
    if (!m_applet) {
        return;
    }

    disconnect(m_hoverTimer, SIGNAL(timeout()), this, SLOT(hoverTimeout()));
    disconnect(m_leaveTimer, SIGNAL(timeout()), this, SLOT(leaveTimeout()));
    m_applet->disconnect(this);

    m_applet->removeSceneEventFilter(this);

    QRectF appletGeomLocal = m_applet->geometry();
    QPointF center = mapToParent(appletGeomLocal.center());
    QPointF appletPos = QPointF(center.x()-appletGeomLocal.width()/2, center.y()-appletGeomLocal.height()/2);
    m_applet->setPos(appletPos);

    // transform is relative to the applet
    QTransform t;
    t.translate(appletGeomLocal.width()/2, appletGeomLocal.height()/2);
    t.rotateRadians(m_angle);
    t.translate(-appletGeomLocal.width()/2, -appletGeomLocal.height()/2);
    m_applet->setTransform(t);

    m_applet->setParentItem(m_containment);

    m_applet->setZValue(m_zValue);

    m_applet->update(); // re-render the background, now we've transformed the applet

    m_applet = 0;
}

QRectF Plasma::AppletHandle::boundingRect() const
{
    return m_totalRect;
}

QPainterPath AppletHandle::shape() const
{
    //when the containment changes the applet is reset to 0
    if (m_applet) {
        QPainterPath path = PaintUtils::roundedRectangle(m_decorationRect, 10);
        return path.united(m_applet->mapToParent(m_applet->shape()));
    } else {
        return QGraphicsItem::shape();
    }
}

QPainterPath handleRect(const QRectF &rect, int radius, bool onRight)
{
    QPainterPath path;
    if (onRight) {
        // make the left side straight
        path.moveTo(rect.left(), rect.top());                                            // Top left
        path.lineTo(rect.right() - radius, rect.top());                                 // Top side
        path.quadTo(rect.right(), rect.top(),
                    rect.right(), rect.top() + radius);    // Top right corner
        path.lineTo(rect.right(), rect.bottom() - radius); // Right side
        path.quadTo(rect.right(), rect.bottom(),
                    rect.right() - radius, rect.bottom()); // Bottom right corner
        path.lineTo(rect.left(), rect.bottom());           // Bottom side
    } else {
        // make the right side straight
        path.moveTo(QPointF(rect.left(), rect.top() + radius));
        path.quadTo(rect.left(), rect.top(),
                    rect.left() + radius, rect.top());     // Top left corner
        path.lineTo(rect.right(), rect.top());             // Top side
        path.lineTo(rect.right(), rect.bottom());          // Right side
        path.lineTo(rect.left() + radius, rect.bottom());  // Bottom side
        path.quadTo(rect.left(), rect.bottom(),
                    rect.left(), rect.bottom() - radius);  // Bottom left corner
    }

    path.closeSubpath();
    return path;
}

void AppletHandle::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    //kDebug() << m_opacity << m_anim << FadeOut;
    if (qFuzzyCompare(m_opacity + 1.0, 1.0)) {
        if (m_anim == FadeOut) {
            //kDebug() << "WOOOOOOOOO";
            QTimer::singleShot(0, this, SLOT(emitDisappear()));
        }
        return;
    }

    qreal translation;

    if (m_buttonsOnRight) {
        //kDebug() << "translating by" << m_opacity
        //         << (-(1 - m_opacity) * m_rect.width()) << m_rect.width();
        translation = -(1 - m_opacity) * m_rect.width();
    } else {
        translation = (1 - m_opacity) * m_rect.width();
    }

    painter->translate(translation, 0);

    painter->setPen(Qt::NoPen);
    painter->setRenderHints(QPainter::Antialiasing);

    int iconMargin = m_iconSize / 2;

    const QSize pixmapSize(int(m_decorationRect.width()),
                           int(m_decorationRect.height()) + m_iconSize * 4 + 1);
    const QSize iconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    //regenerate our buffer?
    if (m_animId > 0 || !m_backgroundBuffer || m_backgroundBuffer->size() != pixmapSize) {
        QColor transparencyColor = Qt::black;
        transparencyColor.setAlphaF(qMin(m_opacity, qreal(0.99)));

        QLinearGradient g(QPoint(0, 0), QPoint(m_decorationRect.width(), 0));
        //fading out panel
        if (m_rect.height() > qreal(minimumHeight()) * 1.25) {
            if (m_buttonsOnRight) {
                qreal opaquePoint =
                    (m_background->marginSize(LeftMargin) - translation) / m_decorationRect.width();
                //kDebug() << "opaquePoint" << opaquePoint
                //         << m_background->marginSize(LeftMargin) << m_decorationRect.width();
                g.setColorAt(0.0, Qt::transparent);
                g.setColorAt(qMax(0.0, opaquePoint - 0.05), Qt::transparent);
                g.setColorAt(opaquePoint, transparencyColor);
                g.setColorAt(1.0, transparencyColor);
            } else {
                qreal opaquePoint =
                    1 - ((m_background->marginSize(RightMargin) + translation) / m_decorationRect.width());
                g.setColorAt(1.0, Qt::transparent);
                g.setColorAt(opaquePoint, Qt::transparent);
                g.setColorAt(qMax(0.0, opaquePoint - 0.05), transparencyColor);
                g.setColorAt(0.0, transparencyColor);
            }
        //complete panel
        } else {
            g.setColorAt(0.0, transparencyColor);
        }

        m_background->resizeFrame(m_decorationRect.size());

        if (!m_backgroundBuffer || m_backgroundBuffer->size() != pixmapSize) {
            delete m_backgroundBuffer;
            m_backgroundBuffer = new QPixmap(pixmapSize);
        }
        m_backgroundBuffer->fill(Qt::transparent);
        QPainter buffPainter(m_backgroundBuffer);

        m_background->paintFrame(&buffPainter);

        //+1 because otherwise due to rounding errors when rotated could appear one pixel
        //of the icon at the border of the applet
        //QRectF iconRect(QPointF(pixmapSize.width() - m_iconSize + 1, m_iconSize), iconSize);
        QRectF iconRect(QPointF(0, m_decorationRect.height() + 1), iconSize);
        if (m_buttonsOnRight) {
            iconRect.moveLeft(
                pixmapSize.width() - m_iconSize - m_background->marginSize(LeftMargin));
            m_configureIcons->paint(&buffPainter, iconRect, "size-diagonal-tr2bl");
        } else {
            iconRect.moveLeft(m_background->marginSize(RightMargin));
            m_configureIcons->paint(&buffPainter, iconRect, "size-diagonal-tl2br");
        }

        iconRect.translate(0, m_iconSize);
        m_configureIcons->paint(&buffPainter, iconRect, "rotate");

        if (m_applet && m_applet->hasConfigurationInterface()) {
            iconRect.translate(0, m_iconSize);
            m_configureIcons->paint(&buffPainter, iconRect, "configure");
        }
        iconRect.translate(0, m_iconSize);
        m_configureIcons->paint(&buffPainter, iconRect, "close");

        buffPainter.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        //blend the background
        buffPainter.fillRect(m_backgroundBuffer->rect(), g);
        //blend the icons
        //buffPainter.fillRect(QRect(QPoint((int)m_decorationRect.width(), 0), QSize(m_iconSize + 1,
        //                                  (int)m_decorationRect.height())), transparencyColor);
    }

    painter->drawPixmap(m_decorationRect.toRect(), *m_backgroundBuffer,
                        QRect(QPoint(0, 0), m_decorationRect.size().toSize()));

    //XXX this code is duplicated in the next function
    QPointF basePoint = m_rect.topLeft() + QPointF(HANDLE_MARGIN, iconMargin);
    QPointF step = QPointF(0, m_iconSize + iconMargin);
    QPointF separator = step + QPointF(0, iconMargin);
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

    QRectF sourceIconRect(QPointF(0, m_decorationRect.height() + 1), iconSize);
    if (m_buttonsOnRight) {
        sourceIconRect.moveLeft(
            pixmapSize.width() - m_iconSize - m_background->marginSize(LeftMargin));
    } else {
        sourceIconRect.moveLeft(m_background->marginSize(RightMargin));
    }

    if (m_applet && m_applet->aspectRatioMode() != FixedSize) {
        //resize
        painter->drawPixmap(
            QRectF(basePoint + shiftM, iconSize), *m_backgroundBuffer, sourceIconRect);
        basePoint += step;
    }

    //rotate
    sourceIconRect.translate(0, m_iconSize);
    painter->drawPixmap(QRectF(basePoint + shiftR, iconSize), *m_backgroundBuffer, sourceIconRect);

    if (m_applet && m_applet->hasConfigurationInterface()) {
        basePoint += step;
        sourceIconRect.translate(0, m_iconSize);
        painter->drawPixmap(
            QRectF(basePoint + shiftC, iconSize), *m_backgroundBuffer, sourceIconRect);
    }

    //close
    basePoint = m_rect.bottomLeft() + QPointF(HANDLE_MARGIN, 0) - step;
    sourceIconRect.translate(0, m_iconSize);
    painter->drawPixmap(QRectF(basePoint + shiftD, iconSize), *m_backgroundBuffer, sourceIconRect);
}

void AppletHandle::emitDisappear()
{
    emit disappearDone(this);
}

AppletHandle::ButtonType AppletHandle::mapToButton(const QPointF &point) const
{
    int iconMargin = m_iconSize / 2;
    //XXX this code is duplicated in the prev. function
    QPointF basePoint = m_rect.topLeft() + QPointF(HANDLE_MARGIN, iconMargin);
    QPointF step = QPointF(0, m_iconSize + iconMargin);
    QPointF separator = step + QPointF(0, iconMargin);
   //end duplicate code

    QRectF activeArea = QRectF(basePoint, QSizeF(m_iconSize, m_iconSize));

    if (m_applet && m_applet->aspectRatioMode() != FixedSize) {
        if (activeArea.contains(point)) {
            return ResizeButton;
        }
        activeArea.translate(step);
    }

    if (activeArea.contains(point)) {
        return RotateButton;
    }

    if (m_applet && m_applet->hasConfigurationInterface()) {
        activeArea.translate(step);
        if (activeArea.contains(point)) {
            return ConfigureButton;
        }
    }

    activeArea.moveTop(m_rect.bottom() - activeArea.height() - iconMargin);
    if (activeArea.contains(point)) {
        return RemoveButton;
    }

    return MoveButton;
    //return m_applet->mapToParent(m_applet->shape()).contains(point) ? NoButton : MoveButton;
}

void AppletHandle::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    //containment recently switched?
    if (!m_applet) {
        QGraphicsItem::mousePressEvent(event);
        return;
    }

    if (m_pendingFade) {
        //m_pendingFade = false;
        return;
    }

    if (event->button() == Qt::LeftButton) {
        m_pressedButton = mapToButton(event->pos());
        //kDebug() << "button pressed:" << m_pressedButton;
        if (m_pressedButton != NoButton) {
            m_applet->raise();
            m_zValue = m_applet->zValue();
            setZValue(m_zValue);
        }

        if (m_pressedButton == MoveButton) {
            m_pos = pos();
        }

        if (m_pressedButton == ResizeButton || m_pressedButton == RotateButton) {
            m_origAppletCenter = mapToScene(m_applet->geometry().center());
            m_origAppletSize = QPointF(m_applet->size().width(), m_applet->size().height());

            // resize
            if (m_buttonsOnRight) {
                m_resizeStaticPoint = mapToScene(m_applet->geometry().bottomLeft());
            } else {
                m_resizeStaticPoint = mapToScene(m_applet->geometry().bottomRight());
            }
            m_resizeGrabPoint = mapToScene(event->pos());                
            QPointF cursorRelativeToStatic = m_resizeGrabPoint - m_resizeStaticPoint;

            // rotate
            m_rotateAngleOffset = m_angle - _k_pointAngle(mapToScene(event->pos()) - m_origAppletCenter);
        }

        event->accept();

        update();

        //set mousePos to the position in the applet, in screencoords, so it becomes easy
        //to reposition the toplevel view to the correct position.
        QPoint localpos = m_currentView->mapFromScene(m_applet->scenePos());
        m_mousePos = event->screenPos() - m_currentView->mapToGlobal(localpos);

        return;
    }

    QGraphicsItem::mousePressEvent(event);
}

bool AppletHandle::leaveCurrentView(const QPoint &pos) const
{
    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
        if (widget->geometry().contains(pos)) {
            //is this widget a plasma view, a different view then our current one,
            //AND not a dashboardview?
            Plasma::View *v = qobject_cast<Plasma::View *>(widget);
            if (v && v != m_currentView && v->containment() != m_containment) {
                return true;
            }
        }
    }
    return false;
}

void AppletHandle::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //kDebug() << "button pressed:" << m_pressedButton << ", fade pending?" << m_pendingFade;

    if (m_pendingFade) {
        startFading(FadeOut, m_entryPos);
        m_pendingFade = false;
    }

    ButtonType releasedAtButton = mapToButton(event->pos());

    if (m_applet && event->button() == Qt::LeftButton) {
        switch (m_pressedButton) {
        case ConfigureButton:
            //FIXME: Remove this call once the configuration management change was done
            if (m_pressedButton == releasedAtButton) {
                m_applet->showConfigurationInterface();
            }
            break;
        case RemoveButton:
            if (m_pressedButton == releasedAtButton) {
                forceDisappear();
                m_applet->destroy();
            }
            break;
        case MoveButton:
        {
                // test for containment change
                //kDebug() << "testing for containment change, sceneBoundingRect = "
                //         << m_containment->sceneBoundingRect();
                if (!m_containment->sceneBoundingRect().contains(m_applet->scenePos())) {
                    // see which containment it belongs to
                    Corona * corona = qobject_cast<Corona*>(scene());
                    if (corona) {
                        QList<Containment*> containments = corona->containments();
                        for (int i = 0; i < containments.size(); ++i) {
                            QPointF pos;
                            QGraphicsView *v;
                            v = containments[i]->view();
                            if (v) {
                                pos = v->mapToScene(
                                    v->mapFromGlobal(event->screenPos() - m_mousePos));

                                if (containments[i]->sceneBoundingRect().contains(pos)) {
                                    //kDebug() << "new containment = " << containments[i];
                                    //kDebug() << "rect = " << containments[i]->sceneBoundingRect();
                                    // add the applet to the new containment and take it from the old one
                                    //kDebug() << "moving to other containment with position" << pos;;
                                    switchContainment(containments[i], pos);
                                    break;
                                }
                            }
                        }
                    }
                }
            break;
        }
        default:
            break;
        }
    }

    m_pressedButton = NoButton;
    update();
}

qreal _k_distanceForPoint(QPointF point)
{
    return std::sqrt(point.x() * point.x() + point.y() * point.y());
}

qreal _k_pointAngle(QPointF in)
{
    qreal r = sqrt(in.x()*in.x() + in.y()*in.y());
    qreal cosine = in.x()/r;
    qreal sine = in.y()/r;

    if (sine>=0) {
        return acos(cosine);
    } else {
        return -acos(cosine);
    }
}

QPointF _k_rotatePoint(QPointF in, qreal rotateAngle)
{
    qreal r = sqrt(in.x()*in.x() + in.y()*in.y());
    qreal cosine = in.x()/r;
    qreal sine = in.y()/r;

    qreal angle;
    if (sine>=0) {
        angle = acos(cosine);
    } else {
        angle = -acos(cosine);
    }

    qreal newAngle = angle + rotateAngle;
    return QPointF(r*cos(newAngle), r*sin(newAngle));
}

void AppletHandle::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    static const qreal snapAngle = M_PI_2 /* $i 3.14159 / 2.0 */;

    if (!m_applet) {
        QGraphicsItem::mouseMoveEvent(event);
        return;
    }

    //Track how much the mouse has moved.
    QPointF deltaScene  = event->scenePos() - event->lastScenePos();

    if (m_pressedButton == MoveButton) {
        m_pos += deltaScene;
        if (leaveCurrentView(event->screenPos())) {
            Plasma::View *v = Plasma::View::topLevelViewAt(event->screenPos());
            if (v && v != m_currentView) {
                Containment *c = v->containment();
                QPoint pos = v->mapFromGlobal(event->screenPos());
                //we actually have been dropped on another containment, so
                //move there: we have a screenpos, we need a scenepos
                //FIXME how reliable is this transform?
                m_pressedButton = NoButton;
                switchContainment(c, v->mapToScene(pos));
            } else {
                setPos(m_pos);
            }
        } else {
            setPos(m_pos);
        }
    } else if (m_pressedButton == ResizeButton || m_pressedButton == RotateButton) {
        QPointF cursorPoint = mapToScene(event->pos());

        // the code below will adjust these based on the type of operation
        QPointF newSize;
        QPointF newCenter;
        qreal newAngle;

        // get size limits
        QSizeF min = m_applet->minimumSize();
        QSizeF max = m_applet->maximumSize();
        // If the applet doesn't have a minimum size, calculate based on a
        // minimum content area size of 16x16
        if (min.isEmpty()) {
            min = m_applet->boundingRect().size() - m_applet->boundingRect().size();
            min += QSizeF(16, 16);
        }

        if (m_pressedButton == RotateButton) {
            newSize = m_origAppletSize;
            newCenter = m_origAppletCenter;

            QPointF centerRelativePoint = cursorPoint - m_origAppletCenter;
            if (_k_distanceForPoint(centerRelativePoint) < 10) {
                newAngle = m_angle;
            } else {
                qreal cursorAngle = _k_pointAngle(centerRelativePoint);
                newAngle = m_rotateAngleOffset + cursorAngle;
                if (fabs(remainder(newAngle, snapAngle)) < 0.15) {
                    newAngle = newAngle - remainder(newAngle, snapAngle);
                }
            }
        } else {
            // un-rotate screen points so we can read differences of coordinates
            QPointF rStaticPoint = _k_rotatePoint(m_resizeStaticPoint, -m_angle);
            QPointF rCursorPoint = _k_rotatePoint(cursorPoint, -m_angle);
            QPointF rGrabPoint = _k_rotatePoint(m_resizeGrabPoint, -m_angle);

            if (m_buttonsOnRight) {
                newSize = m_origAppletSize + QPointF(rCursorPoint.x() - rGrabPoint.x(), rGrabPoint.y() - rCursorPoint.y());
            } else {
                newSize = m_origAppletSize + QPointF(rGrabPoint.x() - rCursorPoint.x(), rGrabPoint.y() - rCursorPoint.y());
            }

            // if preserving aspect ratio, project the calculated size point to the line
            // theough the origin and the original size point
            if ((m_applet->aspectRatioMode() != Plasma::IgnoreAspectRatio &&
                 !(event->modifiers() & Qt::ControlModifier)) ||
                 (m_applet->aspectRatioMode() == Plasma::IgnoreAspectRatio &&
                  (event->modifiers() & Qt::ControlModifier))) {
                qreal ox = m_origAppletSize.x();
                qreal oy = m_origAppletSize.y();
                qreal sx = newSize.x();
                qreal sy = newSize.y();

                qreal x = ox*(sx*ox+sy*oy)/(ox*ox+oy*oy);
                qreal y = (oy/ox)*x;
                newSize = QPointF(x, y);

                // limit size, preserve ratio
                newSize.rx() = qMin(max.width(), qMax(min.width(), newSize.x()));
                newSize.ry() = newSize.x()*(oy/ox);
                newSize.ry() = qMin(max.height(), qMax(min.height(), newSize.y()));
                newSize.rx() = newSize.y()/(oy/ox);
            } else {
                // limit size
                newSize.rx() = qMin(max.width(), qMax(min.width(), newSize.x()));
                newSize.ry() = qMin(max.height(), qMax(min.height(), newSize.y()));
            }

            // move center such that the static corner remains in the same place
            if (m_buttonsOnRight) {
                newCenter =  _k_rotatePoint(QPointF(rStaticPoint.x() + newSize.x()/2, rStaticPoint.y() - newSize.y()/2), m_angle);
            } else {
                newCenter =  _k_rotatePoint(QPointF(rStaticPoint.x() - newSize.x()/2, rStaticPoint.y() - newSize.y()/2), m_angle);
            }

            newAngle = m_angle;
        }

        // set position of applet handle
        QPointF newHandlePosInScene = newCenter - (m_applet->pos() + newSize/2);
        QPointF newHandlePos = parentItem()->mapFromScene(newHandlePosInScene);
        setPos(newHandlePos);

        // set applet size
        m_applet->resize(newSize.x(), newSize.y());

        // set applet handle rotation - rotate around center of applet
        QTransform t;
        QPointF appletCenter = m_applet->geometry().center();
        t.translate(appletCenter.x(), appletCenter.y());
        t.rotateRadians(newAngle);
        t.translate(-appletCenter.x(), -appletCenter.y());
        setTransform(t);
        m_angle = newAngle;

        m_applet->update();
    } else {
        QGraphicsItem::mouseMoveEvent(event);
    }
}

//pos relative to scene
void AppletHandle::switchContainment(Containment *containment, const QPointF &pos)
{
    if (containment->containmentType() != Containment::PanelContainment) {
        //FIXME assuming everything else behaves like desktop?
        //kDebug() << "desktop";
        m_containment = containment;
    }

    Applet *applet = m_applet;
    m_applet = 0; //make sure we don't try to act on the applet again
    applet->removeSceneEventFilter(this);
    forceDisappear(); //takes care of event filter and killing handle
    applet->disconnect(this); //make sure the applet doesn't tell us to do anything
    applet->setZValue(m_zValue);
    containment->addApplet(applet, containment->mapFromScene(pos));
    update();
}

QVariant AppletHandle::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == ItemPositionHasChanged && m_applet) {
        m_applet->updateConstraints(Plasma::LocationConstraint);
    }
    return QGraphicsItem::itemChange(change, value);
}

void AppletHandle::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);
    //kDebug() << "hover enter";

    //if a disappear was scheduled stop the timer
    if (m_leaveTimer->isActive()) {
        m_leaveTimer->stop();
    }
    // if we're already fading out, fade back in
    else if (m_animId != 0 && m_anim == FadeOut) {
        startFading(FadeIn, m_entryPos, true);
    }
}

void AppletHandle::hoverMoveEvent(QGraphicsSceneHoverEvent *event)
{
    hoverEnterEvent(event);
}

void AppletHandle::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Q_UNUSED(event);

    foreach (QWidget *widget, QApplication::topLevelWidgets()) {
      QMenu *menu = qobject_cast<QMenu*>(widget);
      if (menu && menu->isVisible()) {
          connect(menu, SIGNAL(aboutToHide()), this, SLOT(leaveTimeout()));
          return;
      }
    }


    // if we haven't even showed up yet, remove the handle
    if (m_hoverTimer->isActive()) {
        m_hoverTimer->stop();
        QTimer::singleShot(0, this, SLOT(emitDisappear()));
    } else if (m_pressedButton != NoButton) {
        m_pendingFade = true;
    } else {
        //wait a moment to hide the handle in order to recheck the mouse position
        m_leaveTimer->start();
    }
}

bool AppletHandle::sceneEventFilter(QGraphicsItem *watched, QEvent *event)
{
    if (watched == m_applet && event->type() == QEvent::GraphicsSceneHoverLeave) {
        hoverLeaveEvent(static_cast<QGraphicsSceneHoverEvent*>(event));
    }

    return false;
}

void AppletHandle::fadeAnimation(qreal progress)
{
    //qreal endOpacity = (m_anim == FadeIn) ? 1.0 : -1.0;
    if (m_anim == FadeIn) {
        m_opacity = progress;
    } else {
        m_opacity = 1 - progress;
    }

    //kDebug() << "progress" << progress << "m_opacity" << m_opacity << m_anim << "(" << FadeIn << ")";
    if (qFuzzyCompare(progress, qreal(1.0))) {
        m_animId = 0;
        delete m_backgroundBuffer;
        m_backgroundBuffer = 0;
    }

    update();
}

void AppletHandle::hoverTimeout()
{
    startFading(FadeIn, m_entryPos);
}

void AppletHandle::leaveTimeout()
{
    if (!isUnderMouse()) {
        startFading(FadeOut, m_entryPos);
    }
}

void AppletHandle::appletDestroyed()
{
    m_applet = 0;
}

void AppletHandle::appletResized()
{
    prepareGeometryChange();
    calculateSize();
    update();
}

void AppletHandle::setHoverPos(const QPointF &hoverPos)
{
    m_entryPos = hoverPos;
}

void AppletHandle::startFading(FadeType anim, const QPointF &hoverPos, bool preserveSide)
{
    if (m_animId != 0) {
        Animator::self()->stopCustomAnimation(m_animId);
    }

    m_entryPos = hoverPos;
    qreal time = 100;

    if (!m_applet) {
        m_anim = FadeOut;
        fadeAnimation(1.0);
        return;
    }

    if (anim == FadeIn) {
        //kDebug() << m_entryPos.x() << m_applet->pos().x();
        prepareGeometryChange();
        bool wasOnRight = m_buttonsOnRight;
        if (!preserveSide) {
            m_buttonsOnRight = m_entryPos.x() > (m_applet->size().width() / 2);
        }
        calculateSize();
        QPolygonF region = mapToParent(m_rect).intersected(parentWidget()->boundingRect());
        //kDebug() << region << m_rect << mapToParent(m_rect) << parentWidget()->boundingRect();
        if (region != mapToParent(m_rect)) {
            // switch sides
            //kDebug() << "switch sides";
            m_buttonsOnRight = !m_buttonsOnRight;
            calculateSize();
            QPolygonF region2 = mapToParent(m_rect).intersected(parentWidget()->boundingRect());
            if (region2 != mapToParent(m_rect)) {
                // ok, both sides failed to be perfect... which one is more perfect?
                QRectF f1 = region.boundingRect();
                QRectF f2 = region2.boundingRect();
                //kDebug() << "still not a perfect world"
                //         << f2.width() << f2.height() << f1.width() << f1.height();
                if ((f2.width() * f2.height()) < (f1.width() * f1.height())) {
                    //kDebug() << "we did better the first time";
                    m_buttonsOnRight = !m_buttonsOnRight;
                    calculateSize();
                }
            }
        }

        if (wasOnRight != m_buttonsOnRight &&
            m_anim == FadeIn &&
            anim == FadeIn &&
            m_opacity <= 1) {
            m_opacity = 0.0;
        }

        time *= 1.0 - m_opacity;
    } else {
        time *= m_opacity;
    }

    m_anim = anim;
    //kDebug() << "animating for " << time << "ms";
    m_animId = Animator::self()->customAnimation(80 * (time / 1000.0), (int)time,
                                                 Animator::EaseInCurve, this, "fadeAnimation");
}

void AppletHandle::forceDisappear()
{
    setAcceptsHoverEvents(false);
    startFading(FadeOut, m_entryPos);
}

int AppletHandle::minimumHeight()
{
    int iconMargin = m_iconSize / 2;
    int requiredHeight =  iconMargin  + //first margin
                          (m_iconSize + iconMargin) * 4 + //XXX remember to update this if the number of buttons changes
                          iconMargin ;  //blank space before the close button

    if (m_applet && m_applet->hasConfigurationInterface()) {
        requiredHeight += (m_iconSize + iconMargin);
    }

    return requiredHeight;
}

void AppletHandle::calculateSize()
{
    KIconLoader *iconLoader = KIconLoader::global();
    //m_iconSize = iconLoader->currentSize(KIconLoader::Small); //does not work with double sized icon
    m_iconSize = iconLoader->loadIcon("transform-scale", KIconLoader::Small).width(); //workaround

    int handleHeight = qMax(minimumHeight(), int(m_applet->contentsRect().height() * 0.8));
    int handleWidth = m_iconSize + 2 * HANDLE_MARGIN;
    int top =
        m_applet->contentsRect().top() + (m_applet->contentsRect().height() - handleHeight) / 2.0;

    qreal marginLeft, marginTop, marginRight, marginBottom;
    m_background->getMargins(marginLeft, marginTop, marginRight, marginBottom);

    if (m_buttonsOnRight) {
        //put the rect on the right of the applet
        m_rect = QRectF(m_applet->size().width(), top, handleWidth, handleHeight);
    } else {
        //put the rect on the left of the applet
        m_rect = QRectF(-handleWidth, top, handleWidth, handleHeight);
    }

    if (m_applet->contentsRect().height() > qreal(minimumHeight()) * 1.25) {
        int addedMargin = marginLeft / 2;

        // now we check to see if the shape is smaller than the contents,
        // and that the shape is not just the bounding rect; in those cases
        // we have a shaped guy and we draw a full panel;
        // TODO: allow applets to mark when they have translucent areas and
        //       should therefore skip this test?
        if (!m_applet->shape().contains(m_applet->contentsRect())) {
            QPainterPath p;
            p.addRect(m_applet->boundingRect());
            if (m_applet->shape() != p) {
                addedMargin = m_applet->contentsRect().width() / 2;
            }
        }

        if (m_buttonsOnRight) {
            marginLeft += addedMargin;
        } else {
            marginRight += addedMargin;
        }
    }

    m_rect = m_applet->mapToParent(m_rect).boundingRect();
    m_decorationRect = m_rect.adjusted(-marginLeft, -marginTop, marginRight, marginBottom);
    m_totalRect = m_decorationRect.united(m_applet->geometry());
}

} // Plasma Namespace

#include "applethandle_p.moc"

