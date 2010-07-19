/*
 *   Copyright 2009 Marco Martin <notmart@gmail.com>
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

#include "scrollwidget.h"

#include <cmath>

//Qt
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsGridLayout>
#include <QGraphicsScene>
#include <QApplication>
#include <QKeyEvent>
#include <QWidget>
#include <QTimer>
#include <QTime>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>

//KDE
#include <kmimetype.h>
#include <kdebug.h>
#include <kglobalsettings.h>
#include <kiconloader.h>

//Plasma
#include <plasma/widgets/scrollbar.h>
#include <plasma/widgets/svgwidget.h>
#include <plasma/animator.h>
#include <plasma/svg.h>

#define DEBUG 0

/*
  The flicking code is largely based on the behavior of
  the flickable widget in QDeclerative so porting between
  the two should preserve the behavior.
  The code that figures out velocity could use some
  improvements, in particular IGNORE_SUSPICIOUS_MOVES
  is a hack that shouldn't be necessary.
 */

//XXX fixme
//    we use a timer between move events to figure out
//    the velocity of a move, but sometimes we're getting move
//    events with big positional changes with no break
//    in between them, which causes us to compute
//    huge velocities. this define just filters out
//    events which come at insanly small time intervals.
//    at some point we need to figure out how to do it properly
#define IGNORE_SUSPICIOUS_MOVES 1

// FlickThreshold determines how far the "mouse" must have moved
// before we perform a flick.
static const int FlickThreshold = 20;


static const qreal MinimumFlickVelocity = 200;
static const qreal MaxVelocity = 2000;

// time it takes the widget to flick back to its
//  bounds when overshot
static const qreal FixupDuration = 600;

namespace Plasma
{

class ScrollWidgetPrivate
{
public:
    enum Gesture {
        GestureNone = 0,
        GestureUndefined,
        GestureScroll,
        GestureZoom
    };

    ScrollWidgetPrivate(ScrollWidget *parent)
        : q(parent),
          topBorder(0),
          bottomBorder(0),
          leftBorder(0),
          rightBorder(0),
          dragging(false),
          multitouchGesture(GestureNone)
    {
    }

    ~ScrollWidgetPrivate()
    {
    }

    void commonConstructor()
    {
        q->setFocusPolicy(Qt::StrongFocus);
        q->setFiltersChildEvents(true);
        layout = new QGraphicsGridLayout(q);
        q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout->setContentsMargins(1, 1, 1, 1);
        scrollingWidget = new QGraphicsWidget(q);
        scrollingWidget->installEventFilter(q);
        layout->addItem(scrollingWidget, 0, 0);
        borderSvg = new Plasma::Svg(q);
        borderSvg->setImagePath("widgets/scrollwidget");

        adjustScrollbarsTimer = new QTimer(q);
        adjustScrollbarsTimer->setSingleShot(true);
        QObject::connect(adjustScrollbarsTimer, SIGNAL(timeout()), q, SLOT(adjustScrollbars()));

        verticalScrollBarPolicy = Qt::ScrollBarAsNeeded;
        verticalScrollBar = new Plasma::ScrollBar(q);
        verticalScrollBar->setFocusPolicy(Qt::NoFocus);
        layout->addItem(verticalScrollBar, 0, 1);
        verticalScrollBar->nativeWidget()->setMinimum(0);
        verticalScrollBar->nativeWidget()->setMaximum(100);
        QObject::connect(verticalScrollBar, SIGNAL(valueChanged(int)), q, SLOT(verticalScroll(int)));

        horizontalScrollBarPolicy = Qt::ScrollBarAsNeeded;
        horizontalScrollBar = new Plasma::ScrollBar(q);
        verticalScrollBar->setFocusPolicy(Qt::NoFocus);
        horizontalScrollBar->setOrientation(Qt::Horizontal);
        layout->addItem(horizontalScrollBar, 1, 0);
        horizontalScrollBar->nativeWidget()->setMinimum(0);
        horizontalScrollBar->nativeWidget()->setMaximum(100);
        QObject::connect(horizontalScrollBar, SIGNAL(valueChanged(int)), q, SLOT(horizontalScroll(int)));

        flickAnimationX = 0;
        flickAnimationY = 0;
        fixupAnimation.groupX = 0;
        fixupAnimation.startX = 0;
        fixupAnimation.endX = 0;
        fixupAnimation.groupY = 0;
        fixupAnimation.startY = 0;
        fixupAnimation.endY = 0;
        fixupAnimation.snapX = 0;
        fixupAnimation.snapY = 0;
        directMoveAnimation = 0;
        stealEvent = false;
        hasOvershoot = true;

        alignment = Qt::AlignLeft | Qt::AlignTop;

        hasContentsProperty = false;
        hasOffsetProperty = false;
        hasXProperty = false;
        hasYProperty = false;
    }

    void adjustScrollbars()
    {
        if (!widget) {
            return;
        }

        verticalScrollBar->nativeWidget()->setMaximum(qMax(0, int((widget.data()->size().height() - scrollingWidget->size().height())/10)));
        verticalScrollBar->nativeWidget()->setPageStep(int(scrollingWidget->size().height())/10);

        if (verticalScrollBarPolicy == Qt::ScrollBarAlwaysOff ||
            verticalScrollBar->nativeWidget()->maximum() == 0) {
            if (layout->count() > 2 && layout->itemAt(2) == verticalScrollBar) {
                layout->removeAt(2);
            } else if (layout->count() > 1 && layout->itemAt(1) == verticalScrollBar) {
                layout->removeAt(1);
            }
            verticalScrollBar->hide();
        } else if (!verticalScrollBar->isVisible()) {
            layout->addItem(verticalScrollBar, 0, 1);
            verticalScrollBar->show();
        }

        horizontalScrollBar->nativeWidget()->setMaximum(qMax(0, int((widget.data()->size().width() - scrollingWidget->size().width())/10)));
        horizontalScrollBar->nativeWidget()->setPageStep(int(scrollingWidget->size().width())/10);

        if (horizontalScrollBarPolicy == Qt::ScrollBarAlwaysOff ||
            horizontalScrollBar->nativeWidget()->maximum() == 0) {
            if (layout->count() > 2 && layout->itemAt(2) == horizontalScrollBar) {
                layout->removeAt(2);
            } else if (layout->count() > 1 && layout->itemAt(1) == horizontalScrollBar) {
                layout->removeAt(1);
            }
            horizontalScrollBar->hide();
        } else if (!horizontalScrollBar->isVisible()) {
            layout->addItem(horizontalScrollBar, 1, 0);
            horizontalScrollBar->show();
        }

         if (widget && !topBorder && widget.data()->size().height() > q->size().height()) {
            topBorder = new Plasma::SvgWidget(q);
            topBorder->setSvg(borderSvg);
            topBorder->setElementID("border-top");
            topBorder->setZValue(900);
            topBorder->resize(topBorder->effectiveSizeHint(Qt::PreferredSize));
            topBorder->show();
            bottomBorder = new Plasma::SvgWidget(q);
            bottomBorder->setSvg(borderSvg);
            bottomBorder->setElementID("border-bottom");
            bottomBorder->setZValue(900);
            bottomBorder->resize(bottomBorder->effectiveSizeHint(Qt::PreferredSize));
            bottomBorder->show();
        } else if (topBorder && widget && widget.data()->size().height() <= q->size().height()) {
            //FIXME: in some cases topBorder->deleteLater() is deleteNever(), why?
            topBorder->hide();
            bottomBorder->hide();
            topBorder->deleteLater();
            bottomBorder->deleteLater();
            topBorder = 0;
            bottomBorder = 0;
        }


        if (widget && !leftBorder && widget.data()->size().width() > q->size().width()) {
            leftBorder = new Plasma::SvgWidget(q);
            leftBorder->setSvg(borderSvg);
            leftBorder->setElementID("border-left");
            leftBorder->setZValue(900);
            leftBorder->resize(leftBorder->effectiveSizeHint(Qt::PreferredSize));
            leftBorder->show();
            rightBorder = new Plasma::SvgWidget(q);
            rightBorder->setSvg(borderSvg);
            rightBorder->setElementID("border-right");
            rightBorder->setZValue(900);
            rightBorder->resize(rightBorder->effectiveSizeHint(Qt::PreferredSize));
            rightBorder->show();
        } else if (leftBorder && widget && widget.data()->size().width() <= q->size().width()) {
            leftBorder->hide();
            rightBorder->hide();
            leftBorder->deleteLater();
            rightBorder->deleteLater();
            leftBorder = 0;
            rightBorder = 0;
        }

        layout->activate();

        if (topBorder) {
            topBorder->resize(q->size().width(), topBorder->size().height());
            bottomBorder->resize(q->size().width(), bottomBorder->size().height());
            bottomBorder->setPos(0, q->size().height() - topBorder->size().height());
        }
        if (leftBorder) {
            leftBorder->resize(leftBorder->size().width(), q->size().height());
            rightBorder->resize(rightBorder->size().width(), q->size().height());
            rightBorder->setPos(q->size().width() - rightBorder->size().width(), 0);
        }

        QSizeF widgetSize = widget.data()->size();
        if (widget.data()->sizePolicy().expandingDirections() & Qt::Horizontal) {
            //keep a 1 pixel border
            widgetSize.setWidth(scrollingWidget->size().width()-borderSize);
        }
        if (widget.data()->sizePolicy().expandingDirections() & Qt::Vertical) {
            widgetSize.setHeight(scrollingWidget->size().height()-borderSize);
        }
        widget.data()->resize(widgetSize);

        adjustClipping();
    }

    void verticalScroll(int value)
    {
        if (!widget) {
            return;
        }

        if (!dragging) {
            widget.data()->setPos(QPoint(widget.data()->pos().x(), -value*10));
        }
    }

    void horizontalScroll(int value)
    {
        if (!widget) {
            return;
        }

        if (!dragging) {
            widget.data()->setPos(QPoint(-value*10, widget.data()->pos().y()));
        }
    }

    void adjustClipping()
    {
        if (!widget) {
            return;
        }

        const bool clip = widget.data()->size().width() > scrollingWidget->size().width() || widget.data()->size().height() > scrollingWidget->size().height();

        scrollingWidget->setFlag(QGraphicsItem::ItemClipsChildrenToShape, clip);
    }

    qreal overShootDistance(qreal velocity, qreal size) const
    {
        if (MaxVelocity <= 0)
            return 0.0;

        velocity = qAbs(velocity);
        if (velocity > MaxVelocity)
            velocity = MaxVelocity;
        qreal dist = size / 4 * velocity / MaxVelocity;
        return dist;
    }

    void animateMoveTo(const QPointF &pos)
    {
        qreal duration = 800;
        QPointF start = q->scrollPosition();
        QSizeF threshold = q->viewportGeometry().size();
        QPointF diff = pos - start;

        //reduce if it's within the viewport
        if (qAbs(diff.x()) < threshold.width() ||
            qAbs(diff.y()) < threshold.height())
            duration /= 2;

        fixupAnimation.groupX->stop();
        fixupAnimation.groupY->stop();
        fixupAnimation.snapX->stop();
        fixupAnimation.snapY->stop();

        directMoveAnimation->setStartValue(start);
        directMoveAnimation->setEndValue(pos);
        directMoveAnimation->setDuration(duration);
        directMoveAnimation->start();
    }

    void flick(QPropertyAnimation *anim,
               qreal velocity,
               qreal val,
               qreal minExtent,
               qreal maxExtent,
               qreal size)
    {
        qreal deceleration = 500;
        qreal maxDistance = -1;
        qreal target = 0;
        // -ve velocity means list is moving up
        if (velocity > 0) {
            if (val < minExtent)
                maxDistance = qAbs(minExtent - val + (hasOvershoot?overShootDistance(velocity,size):0));
            target = minExtent;
            deceleration = -deceleration;
        } else {
            if (val > maxExtent)
                maxDistance = qAbs(maxExtent - val) + (hasOvershoot?overShootDistance(velocity,size):0);
            target = maxExtent;
        }
        if (maxDistance > 0) {
            qreal v = velocity;
            if (MaxVelocity != -1 && MaxVelocity < qAbs(v)) {
                if (v < 0)
                    v = -MaxVelocity;
                else
                    v = MaxVelocity;
            }
            qreal duration = qAbs(v / deceleration);
            qreal diffY = v * duration + (0.5  * deceleration * duration * duration);
            qreal startY = val;
 
            qreal endY = startY + diffY;

            if (velocity > 0) {
                if (endY > target)
                    endY = startY + maxDistance;
            } else {
                if (endY < target)
                    endY = startY - maxDistance;
            }
            duration = qAbs((endY-startY)/ (-v/2));

            if (hasYProperty) {
                startY = -startY;
                endY = -endY;
            }


#if DEBUG
            qDebug()<<"XXX velocity = "<<v <<", target = "<< target
                    <<", maxDist = "<<maxDistance;
            qDebug()<<"duration = "<<duration<<" secs, ("
                    << (duration * 1000) <<" msecs)";
            qDebug()<<"startY = "<<startY;
            qDebug()<<"endY = "<<endY;
            qDebug()<<"overshoot = "<<overShootDistance(v, size);
            qDebug()<<"avg velocity = "<< ((endY-startY)/duration);
#endif

            anim->setStartValue(startY);
            anim->setEndValue(endY);
            anim->setDuration(duration * 1000);
            anim->start();
        } else {
            if (anim == flickAnimationX)
                fixupX();
            else
                fixupY();
        }
    }
    void flickX(qreal velocity)
    {
        flick(flickAnimationX, velocity, widgetX(), minXExtent(), maxXExtent(),
              q->viewportGeometry().width());
    }
    void flickY(qreal velocity)
    {
        flick(flickAnimationY, velocity, widgetY(),minYExtent(), maxYExtent(),
              q->viewportGeometry().height());
    }
    void fixup(QAnimationGroup *group,
               QPropertyAnimation *start, QPropertyAnimation *end,
               qreal val, qreal minExtent, qreal maxExtent)
    {
        if (val > minExtent || maxExtent > minExtent) {
            if (!qFuzzyCompare(val, minExtent)) {
                if (FixupDuration) {
                    //TODO: we should consider the case where there is one axis available not the other
                    if (hasXProperty && hasYProperty) {
                        val = -val;
                        minExtent = -minExtent;
                    }
                    qreal dist = minExtent - val;
                    start->setStartValue(val);
                    start->setEndValue(minExtent - dist/2);
                    end->setStartValue(minExtent - dist/2);
                    end->setEndValue(minExtent);
                    start->setDuration(FixupDuration/4);
                    end->setDuration(3*FixupDuration/4);
                    group->start();
                } else {
                    QObject *obj = start->targetObject();
                    obj->setProperty(start->propertyName(), minExtent);
                }
            }
        } else if (val < maxExtent) {
            if (FixupDuration) {
                if (hasXProperty && hasYProperty) {
                    val = -val;
                    maxExtent = -maxExtent;
                }
                qreal dist = maxExtent - val;
                start->setStartValue(val);
                start->setEndValue(maxExtent - dist/2);
                end->setStartValue(maxExtent - dist/2);
                end->setEndValue(maxExtent);
                start->setDuration(FixupDuration/4);
                end->setDuration(3*FixupDuration/4);
                group->start();
            } else {
                QObject *obj = start->targetObject();
                obj->setProperty(start->propertyName(), maxExtent);
            }
        } else if (end == fixupAnimation.endX && snapSize.width() > 1 &&
                   q->contentsSize().width() > q->viewportGeometry().width()) {
            int target = snapSize.width() * round(val/snapSize.width());
            fixupAnimation.snapX->setStartValue(val);
            fixupAnimation.snapX->setEndValue(target);
            fixupAnimation.snapX->setDuration(FixupDuration);
            fixupAnimation.snapX->start();
        } else if (end == fixupAnimation.endY && snapSize.height() > 1 &&
                   q->contentsSize().height() > q->viewportGeometry().height()) {
            int target = snapSize.height() * round(val/snapSize.height());
            fixupAnimation.snapY->setStartValue(val);
            fixupAnimation.snapY->setEndValue(target);
            fixupAnimation.snapY->setDuration(FixupDuration);
            fixupAnimation.snapY->start();
        }
    }
    void fixupX()
    {
        fixup(fixupAnimation.groupX, fixupAnimation.startX, fixupAnimation.endX,
              widgetX(), minXExtent(), maxXExtent());
    }
    void fixupY()
    {
        fixup(fixupAnimation.groupY, fixupAnimation.startY, fixupAnimation.endY,
              widgetY(), minYExtent(), maxYExtent());
    }

    void makeRectVisible()
    {
        if (!widget) {
            return;
        }

        QRectF viewRect = scrollingWidget->boundingRect();
        //ensure the rect is not outside the widget bounding rect
        QRectF mappedRect = QRectF(QPointF(qBound((qreal)0.0, rectToBeVisible.x(), widget.data()->size().width() - rectToBeVisible.width()),
                                           qBound((qreal)0.0, rectToBeVisible.y(), widget.data()->size().height() - rectToBeVisible.height())),
                                           rectToBeVisible.size());
        mappedRect = widget.data()->mapToItem(scrollingWidget, mappedRect).boundingRect();

        if (viewRect.contains(mappedRect)) {
            return;
        }

        QPointF delta(0, 0);

        if (mappedRect.top() < 0) {
            delta.setY(-mappedRect.top());
        } else if  (mappedRect.bottom() > viewRect.bottom()) {
            delta.setY(viewRect.bottom() - mappedRect.bottom());
        }

        if (mappedRect.left() < 0) {
            delta.setX(-mappedRect.left());
        } else if  (mappedRect.right() > viewRect.right()) {
            delta.setX(viewRect.right() - mappedRect.right());
        }

        animateMoveTo(q->scrollPosition() - delta);
    }

    void makeItemVisible(QGraphicsItem *itemToBeVisible)
    {
        if (!widget) {
            return;
        }

        QRectF rect(widget.data()->mapFromScene(itemToBeVisible->scenePos()), itemToBeVisible->boundingRect().size());
        rectToBeVisible = rect;

        makeRectVisible();
    }

    void makeItemVisible()
    {
        if (widgetToBeVisible) {
            makeItemVisible(widgetToBeVisible.data());
        }
    }

    void stopAnimations()
    {
        flickAnimationX->stop();
        flickAnimationY->stop();
        fixupAnimation.groupX->stop();
        fixupAnimation.groupY->stop();
    }

    void setWidgetX(qreal x)
    {
        if (hasXProperty) {
            widget.data()->setProperty("scrollPositionX", -x);
        } else
            widget.data()->setX(x);
    }
    void setWidgetY(qreal y)
    {
        if (hasYProperty) {
            widget.data()->setProperty("scrollPositionY", -y);
        } else
            widget.data()->setY(y);
    }
    qreal widgetX() const
    {
        if (hasXProperty) {
            return -widget.data()->property("scrollPositionX").toReal();
        } else
            return widget.data()->x();
    }
    qreal widgetY() const
    {
        if (hasYProperty) {
            return -widget.data()->property("scrollPositionY").toReal();
        } else
            return widget.data()->y();
    }

    void handleKeyPressEvent(QKeyEvent *event)
    {
        if (!widget.data())
            return;

        QPointF start = q->scrollPosition();
        QPointF end = start;

        qreal step = 100;

        switch (event->key()) {
        case Qt::Key_Left:
            if (canXFlick()) {
                end += QPointF(-step, 0);
            }
            break;
        case Qt::Key_Right:
            if (canXFlick()) {
                end += QPointF(step, 0);
            }
            break;
        case Qt::Key_Up:
            if (canYFlick()) {
                end += QPointF(0, -step);
            }
            break;
        case Qt::Key_Down:
            if (canYFlick()) {
                end += QPointF(0, step);
            }
            break;
        default:
            break;
        }

        fixupAnimation.groupX->stop();
        fixupAnimation.groupY->stop();
        fixupAnimation.snapX->stop();
        fixupAnimation.snapY->stop();
        directMoveAnimation->setStartValue(start);
        directMoveAnimation->setEndValue(end);
        directMoveAnimation->setDuration(200);
        directMoveAnimation->start();
    }

    void handleMousePressEvent(QGraphicsSceneMouseEvent *event)
    {
        lastPos = QPoint();
        lastPosTime = QTime::currentTime();
        pressPos = event->scenePos();
        pressScrollPos = -q->scrollPosition();
        pressTime = QTime::currentTime();
        velocity = QPointF();
        stopAnimations();
    }

    void handleMouseMoveEvent(QGraphicsSceneMouseEvent *event)
    {
        if (lastPosTime.isNull())
            return;
        bool rejectY = false;
        bool rejectX = false;
        bool moved = false;

        if (canYFlick()) {
            int dy = int(event->scenePos().y() - pressPos.y());
            if (qAbs(dy) > KGlobalSettings::dndEventDelay() || elapsed(pressTime) > 200) {
                qreal newY = dy + pressScrollPos.y();
                const qreal minY = minYExtent();
                const qreal maxY = maxYExtent();
                if (newY > minY)
                    newY = minY + (newY - minY) / 2;
                if (newY < maxY && maxY - minY <= 0)
                    newY = maxY + (newY - maxY) / 2;
                if (!hasOvershoot && (newY > minY || newY < maxY)) {
                    if (newY > minY)
                        newY = minY;
                    else if (newY < maxY)
                        newY = maxY;
                    else
                        rejectY = true;
                }
                if (!rejectY && stealEvent) {
                    setWidgetY(qRound(newY));
                    moved = true;
                }
                if (qAbs(dy) > KGlobalSettings::dndEventDelay())
                    stealEvent = true;
            }
        }

        if (canXFlick()) {
            int dx = int(event->scenePos().x() - pressPos.x());
            if (qAbs(dx) > KGlobalSettings::dndEventDelay() || elapsed(pressTime) > 200) {
                qreal newX = dx + pressScrollPos.x();
                const qreal minX = minXExtent();
                const qreal maxX = maxXExtent();
                if (newX > minX)
                    newX = minX + (newX - minX) / 2;
                if (newX < maxX && maxX - minX <= 0)
                    newX = maxX + (newX - maxX) / 2;
                if (!hasOvershoot && (newX > minX || newX < maxX)) {
                    if (newX > minX)
                        newX = minX;
                    else if (newX < maxX)
                        newX = maxX;
                    else
                        rejectX = true;
                }
                if (!rejectX && stealEvent) {
                    setWidgetX(qRound(newX));
                    moved = true;
                }

                if (qAbs(dx) > KGlobalSettings::dndEventDelay())
                    stealEvent = true;
            }
        }

        if (!lastPos.isNull()) {
            qreal msecs = qreal(restart(lastPosTime));
            qreal elapsed =  msecs / 1000.;
#if IGNORE_SUSPICIOUS_MOVES
            if (msecs > 3) {
#endif
            if (elapsed <= 0)
                elapsed = 1;
            if (canYFlick()) {
                qreal diff = event->scenePos().y() - lastPos.y();
                // average to reduce the effect of spurious moves
                velocity.setY( velocity.y() + (diff / elapsed) );
                velocity.setY( velocity.y() / 2 );
            }

            if (canXFlick()) {
                qreal diff = event->scenePos().x() - lastPos.x();
                // average to reduce the effect of spurious moves
                velocity.setX( velocity.x() + (diff / elapsed) );
                velocity.setX( velocity.x() / 2 );
            }
#if IGNORE_SUSPICIOUS_MOVES
            }
#endif
        }

        if (rejectX) velocity.setX(0);
        if (rejectY) velocity.setY(0);

        lastPos = event->scenePos();
    }

    void handleMouseReleaseEvent(QGraphicsSceneMouseEvent *event)
    {
        stealEvent = false;
        if (lastPosTime.isNull())
            return;

        if (elapsed(lastPosTime) > 100) {
            // if we drag then pause before release we should not cause a flick.
            velocity = QPointF();
        }

        if (qAbs(velocity.y()) > 10 &&
            qAbs(event->scenePos().y() - pressPos.y()) > FlickThreshold) {
            qreal vVelocity = velocity.y();
            // Minimum velocity to avoid annoyingly slow flicks.
            if (qAbs(vVelocity) < MinimumFlickVelocity)
                vVelocity = vVelocity < 0 ? -MinimumFlickVelocity : MinimumFlickVelocity;
            flickY(vVelocity);
        } else {
            fixupY();
        }

        if (qAbs(velocity.x()) > 10 &&
            qAbs(event->scenePos().x() - pressPos.x()) > FlickThreshold) {
            qreal hVelocity = velocity.x();
            // Minimum velocity to avoid annoyingly slow flicks.
            if (qAbs(hVelocity) < MinimumFlickVelocity)
                hVelocity = hVelocity < 0 ? -MinimumFlickVelocity : MinimumFlickVelocity;
            flickX(hVelocity);
        } else {
            fixupX();
        }

        lastPosTime = QTime();
    }

    void handleWheelEvent(QGraphicsSceneWheelEvent *event)
    {
        if (!widget.data())
            return;

        QPointF start = q->scrollPosition();
        QPointF end = start;

        //At some point we should switch to
        // step = QApplication::wheelScrollLines() *
        //      (event->delta()/120) *
        //      scrollBar->singleStep();
        // which gives us exactly the number of lines to scroll but the issue
        // is that at this point we don't have any clue what a "line" is and if
        // we make it a pixel then scrolling by 3 (default) pixels will be
        // very painful
        qreal step = -event->delta()/3;

        //ifthe widget can scroll in a single axis and the wheel is the other one, scroll the other one
        Qt::Orientation orientation = event->orientation();
        if (orientation == Qt::Vertical) {
            if (!canYFlick() && canXFlick()) {
                end += QPointF(step, 0);
            } else if (canYFlick()) {
                end += QPointF(0, step);
            } else {
                return;
            }
        } else {
            if (canYFlick() && !canXFlick()) {
                end += QPointF(0, step);
            } else if (canXFlick()) {
                end += QPointF(step, 0);
            } else {
                return;
            }
        }

        fixupAnimation.groupX->stop();
        fixupAnimation.groupY->stop();
        fixupAnimation.snapX->stop();
        fixupAnimation.snapY->stop();
        directMoveAnimation->setStartValue(start);
        directMoveAnimation->setEndValue(end);
        directMoveAnimation->setDuration(200);
        directMoveAnimation->start();
    }

    qreal minXExtent() const
    {
        if (alignment & Qt::AlignLeft)
            return 0;
        else {
            qreal vWidth = q->viewportGeometry().width();
            qreal cWidth = q->contentsSize().width();
            if (cWidth < vWidth) {
                if (alignment & Qt::AlignRight)
                    return  vWidth - cWidth;
                else if (alignment & Qt::AlignHCenter)
                    return vWidth / 2 - cWidth / 2;
            }
        }

        return 0;
    }

    qreal maxXExtent() const
    {
        return q->viewportGeometry().width() -
            q->contentsSize().width();
    }

    qreal minYExtent() const
    {
        if (alignment & Qt::AlignTop)
            return 0;
        else {
            qreal vHeight = q->viewportGeometry().height();
            qreal cHeight = q->contentsSize().height();
            if (cHeight < vHeight) {
                if (alignment & Qt::AlignBottom)
                    return  vHeight - cHeight;
                else if (alignment & Qt::AlignVCenter)
                    return vHeight / 2 - cHeight / 2;
            }
        }

        return 0;
    }

    qreal maxYExtent() const
    {
        return q->viewportGeometry().height() -
            q->contentsSize().height();
    }

    bool canXFlick() const
    {
        //make the thing feel quite "fixed" don't permit to flick when the contents size is less than the viewport
        return q->contentsSize().width() + borderSize > q->viewportGeometry().width();
    }

    bool canYFlick() const
    {
        return q->contentsSize().height() + borderSize > q->viewportGeometry().height();
    }

    int elapsed(const QTime &t) const
    {
        int n = t.msecsTo(QTime::currentTime());
        if (n < 0) // passed midnight
            n += 86400 * 1000;
        return n;
    }

    int restart(QTime &t) const
    {
        QTime time = QTime::currentTime();
        int n = t.msecsTo(time);
        if (n < 0) // passed midnight
            n += 86400*1000;
        t = time;
        return n;
    }

    void createFlickAnimations()
    {
        if (widget.data()) {
            QString xProp = QString::fromLatin1("x");
            QString yProp = QString::fromLatin1("y");

            if (hasXProperty)
                xProp = QString::fromLatin1("scrollPositionX");
            if (hasYProperty)
                yProp = QString::fromLatin1("scrollPositionY");

            flickAnimationX = new QPropertyAnimation(widget.data(),
                                                     xProp.toLatin1(), widget.data());
            flickAnimationY = new QPropertyAnimation(widget.data(),
                                                     yProp.toLatin1(), widget.data());
            QObject::connect(flickAnimationX, SIGNAL(finished()),
                             q, SLOT(fixupX()));
            QObject::connect(flickAnimationY, SIGNAL(finished()),
                             q, SLOT(fixupY()));

            QObject::connect(flickAnimationX,
                             SIGNAL(stateChanged(QAbstractAnimation::State,
                                                 QAbstractAnimation::State)),
                             q, SIGNAL(scrollStateChanged(QAbstractAnimation::State,
                                                          QAbstractAnimation::State)));
            QObject::connect(flickAnimationY,
                             SIGNAL(stateChanged(QAbstractAnimation::State,
                                                 QAbstractAnimation::State)),
                             q, SIGNAL(scrollStateChanged(QAbstractAnimation::State,
                                                          QAbstractAnimation::State)));

            flickAnimationX->setEasingCurve(QEasingCurve::OutCirc);
            flickAnimationY->setEasingCurve(QEasingCurve::OutCirc);


            fixupAnimation.groupX = new QSequentialAnimationGroup(widget.data());
            fixupAnimation.groupY = new QSequentialAnimationGroup(widget.data());
            fixupAnimation.startX  = new QPropertyAnimation(widget.data(),
                                                            xProp.toLatin1(), widget.data());
            fixupAnimation.startY  = new QPropertyAnimation(widget.data(),
                                                            yProp.toLatin1(), widget.data());
            fixupAnimation.endX = new QPropertyAnimation(widget.data(),
                                                         xProp.toLatin1(), widget.data());
            fixupAnimation.endY = new QPropertyAnimation(widget.data(),
                                                         yProp.toLatin1(), widget.data());
            fixupAnimation.groupX->addAnimation(
                fixupAnimation.startX);
            fixupAnimation.groupY->addAnimation(
                fixupAnimation.startY);
            fixupAnimation.groupX->addAnimation(
                fixupAnimation.endX);
            fixupAnimation.groupY->addAnimation(
                fixupAnimation.endY);

            fixupAnimation.startX->setEasingCurve(QEasingCurve::InQuad);
            fixupAnimation.endX->setEasingCurve(QEasingCurve::OutQuint);
            fixupAnimation.startY->setEasingCurve(QEasingCurve::InQuad);
            fixupAnimation.endY->setEasingCurve(QEasingCurve::OutQuint);

            fixupAnimation.snapX = new QPropertyAnimation(widget.data(),
                                                          xProp.toLatin1(), widget.data());
            fixupAnimation.snapY = new QPropertyAnimation(widget.data(),
                                                          yProp.toLatin1(), widget.data());
            fixupAnimation.snapX->setEasingCurve(QEasingCurve::InOutQuad);
            fixupAnimation.snapY->setEasingCurve(QEasingCurve::InOutQuad);

            QObject::connect(fixupAnimation.groupX,
                             SIGNAL(stateChanged(QAbstractAnimation::State,
                                                 QAbstractAnimation::State)),
                             q, SIGNAL(scrollStateChanged(QAbstractAnimation::State,
                                                          QAbstractAnimation::State)));
            QObject::connect(fixupAnimation.groupY,
                             SIGNAL(stateChanged(QAbstractAnimation::State,
                                                 QAbstractAnimation::State)),
                             q, SIGNAL(scrollStateChanged(QAbstractAnimation::State,
                                                          QAbstractAnimation::State)));

            directMoveAnimation = new QPropertyAnimation(q,
                                                         "scrollPosition",
                                                         q);
            QObject::connect(directMoveAnimation, SIGNAL(finished()),
                             q, SLOT(fixupX()));
            QObject::connect(directMoveAnimation, SIGNAL(finished()),
                             q, SLOT(fixupY()));
            QObject::connect(directMoveAnimation,
                             SIGNAL(stateChanged(QAbstractAnimation::State,
                                                 QAbstractAnimation::State)),
                             q, SIGNAL(scrollStateChanged(QAbstractAnimation::State,
                                                          QAbstractAnimation::State)));
            directMoveAnimation->setEasingCurve(QEasingCurve::OutCirc);
        }
    }

    void deleteFlickAnimations()
    {
        if (flickAnimationX)
            flickAnimationX->stop();
        if (flickAnimationY)
            flickAnimationY->stop();
        delete flickAnimationX;
        delete flickAnimationY;
        delete fixupAnimation.groupX;
        delete fixupAnimation.groupY;
        delete directMoveAnimation;
        delete fixupAnimation.snapX;
        delete fixupAnimation.snapY;
    }

    void setScrollX()
    {
        if (horizontalScrollBarPolicy != Qt::ScrollBarAlwaysOff) {
            horizontalScrollBar->blockSignals(true);
            horizontalScrollBar->setValue(-widget.data()->pos().x()/10.);
            horizontalScrollBar->blockSignals(false);
        }
    }

    void setScrollY()
    {
        if (verticalScrollBarPolicy != Qt::ScrollBarAlwaysOff) {
            verticalScrollBar->blockSignals(true);
            verticalScrollBar->setValue(-widget.data()->pos().y()/10.);
            verticalScrollBar->blockSignals(false);
        }
    }

    ScrollWidget *q;
    QGraphicsWidget *scrollingWidget;
    QWeakPointer<QGraphicsWidget> widget;
    Plasma::Svg *borderSvg;
    Plasma::SvgWidget *topBorder;
    Plasma::SvgWidget *bottomBorder;
    Plasma::SvgWidget *leftBorder;
    Plasma::SvgWidget *rightBorder;
    QGraphicsGridLayout *layout;
    ScrollBar *verticalScrollBar;
    Qt::ScrollBarPolicy verticalScrollBarPolicy;
    ScrollBar *horizontalScrollBar;
    Qt::ScrollBarPolicy horizontalScrollBarPolicy;
    QString styleSheet;
    QWeakPointer<QGraphicsWidget> widgetToBeVisible;
    QRectF rectToBeVisible;
    QPointF dragHandleClicked;
    bool dragging;
    QTimer *adjustScrollbarsTimer;
    static const int borderSize = 4;

    QPointF pressPos;
    QPointF pressScrollPos;
    QPointF velocity;
    QPointF lastPos;
    QTime pressTime;
    QTime lastPosTime;
    QPropertyAnimation *flickAnimationX;
    QPropertyAnimation *flickAnimationY;
    struct {
        QAnimationGroup *groupX;
        QPropertyAnimation *startX;
        QPropertyAnimation *endX;

        QAnimationGroup *groupY;
        QPropertyAnimation *startY;
        QPropertyAnimation *endY;

        QPropertyAnimation *snapX;
        QPropertyAnimation *snapY;
    } fixupAnimation;
    QPropertyAnimation *directMoveAnimation;
    QSizeF snapSize;
    bool stealEvent;
    bool hasOvershoot;

    Qt::Alignment alignment;

    Gesture multitouchGesture;

    bool hasContentsProperty;
    bool hasOffsetProperty;
    bool hasXProperty;
    bool hasYProperty;
};


ScrollWidget::ScrollWidget(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      d(new ScrollWidgetPrivate(this))
{
    d->commonConstructor();
}

ScrollWidget::ScrollWidget(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new ScrollWidgetPrivate(this))
{
    d->commonConstructor();
}

ScrollWidget::~ScrollWidget()
{
    delete d;
}

void ScrollWidget::setWidget(QGraphicsWidget *widget)
{
    if (d->widget && d->widget.data() != widget) {
        d->deleteFlickAnimations();
        d->widget.data()->removeEventFilter(this);
        delete d->widget.data();
    }

    d->widget = widget;
    //it's not good it's setting a size policy here, but it's done to be retrocompatible with older applications
    if (widget) {
        d->hasContentsProperty = widget->property("contentsSize").isValid();
        d->hasOffsetProperty = widget->property("scrollPosition").isValid();
        d->hasXProperty = widget->property("scrollPositionX").isValid();
        d->hasYProperty = widget->property("scrollPositionY").isValid();
        d->createFlickAnimations();

        connect(widget, SIGNAL(xChanged()), this, SLOT(setScrollX()));
        connect(widget, SIGNAL(yChanged()), this, SLOT(setScrollY()));
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        widget->setParentItem(d->scrollingWidget);
        widget->setPos(d->minXExtent(), d->minYExtent());
        widget->installEventFilter(this);
        d->adjustScrollbarsTimer->start(200);
    }
}

QGraphicsWidget *ScrollWidget::widget() const
{
    return d->widget.data();
}


void ScrollWidget::setHorizontalScrollBarPolicy(const Qt::ScrollBarPolicy policy)
{
    d->horizontalScrollBarPolicy = policy;
}


Qt::ScrollBarPolicy ScrollWidget::horizontalScrollBarPolicy() const
{
    return d->horizontalScrollBarPolicy;
}


void ScrollWidget::setVerticalScrollBarPolicy(const Qt::ScrollBarPolicy policy)
{
    d->verticalScrollBarPolicy = policy;
}

Qt::ScrollBarPolicy ScrollWidget::verticalScrollBarPolicy() const
{
    return d->verticalScrollBarPolicy;
}

void ScrollWidget::ensureRectVisible(const QRectF &rect)
{
    if (!d->widget) {
        return;
    }

    d->rectToBeVisible = rect;
    d->makeRectVisible();
}

void ScrollWidget::ensureItemVisible(QGraphicsItem *item)
{
    if (!d->widget || !item) {
        return;
    }

    QGraphicsItem *parentOfItem = item->parentItem();
    while (parentOfItem != d->widget.data()) {
        if (!parentOfItem) {
            return;
        }

        parentOfItem = parentOfItem->parentItem();
    }

    //since we can't ensure it'll stay alive we can delay only if it's a qgraphicswidget
    QGraphicsWidget *widget = qgraphicsitem_cast<QGraphicsWidget *>(item);
    if (widget) {
        d->widgetToBeVisible = widget;

        // We need to wait for the parent item to resize...
        QTimer::singleShot(0, this, SLOT(makeItemVisible()));
    } else {
        d->makeItemVisible(item);
    }
}

void ScrollWidget::registerAsDragHandle(QGraphicsWidget *item)
{
    Q_UNUSED(item);
    return;
}

void ScrollWidget::unregisterAsDragHandle(QGraphicsWidget *item)
{
    Q_UNUSED(item);
    return;
}

QRectF ScrollWidget::viewportGeometry() const
{
    QRectF result;
    if (!d->widget) {
        return result;
    }

    return d->scrollingWidget->boundingRect();
}

QSizeF ScrollWidget::contentsSize() const
{
    if (d->widget) {
        if (d->hasContentsProperty) {
            QVariant var = d->widget.data()->property("contentsSize");
            return var.toSizeF();
        } else
            return d->widget.data()->size();
    }
    return QSizeF();
}

void ScrollWidget::setScrollPosition(const QPointF &position)
{
    if (d->widget) {
        if (d->hasOffsetProperty)
            d->widget.data()->setProperty("scrollPosition", position);
        else
            d->widget.data()->setPos(-position.toPoint());
    }
}

QPointF ScrollWidget::scrollPosition() const
{
    if (d->widget) {
        if (d->hasOffsetProperty) {
            QVariant var = d->widget.data()->property("scrollPosition");
            return var.toPointF();
        } else {
            return -d->widget.data()->pos();
        }
    }
    return QPointF();
}

void ScrollWidget::setSnapSize(const QSizeF &size)
{
    d->snapSize = size;
}

QSizeF ScrollWidget::snapSize() const
{
    return d->snapSize;
}

void ScrollWidget::setStyleSheet(const QString &styleSheet)
{
    d->styleSheet = styleSheet;
    d->verticalScrollBar->setStyleSheet(styleSheet);
    d->horizontalScrollBar->setStyleSheet(styleSheet);
}

QString ScrollWidget::styleSheet() const
{
    return d->styleSheet;
}

QWidget *ScrollWidget::nativeWidget() const
{
    return 0;
}

void ScrollWidget::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if (d->widget) {
        d->widget.data()->setFocus();
    }
}


void ScrollWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    if (!d->widget) {
        QGraphicsWidget::resizeEvent(event);
        return;
    }

    d->adjustScrollbarsTimer->start(200);

    //if topBorder exists bottomBorder too
    if (d->topBorder) {
        d->topBorder->resize(event->newSize().width(), d->topBorder->size().height());
        d->bottomBorder->resize(event->newSize().width(), d->bottomBorder->size().height());
        d->bottomBorder->setPos(0, event->newSize().height() - d->bottomBorder->size().height());
    }
    if (d->leftBorder) {
        d->leftBorder->resize(d->leftBorder->size().width(), event->newSize().height());
        d->rightBorder->resize(d->rightBorder->size().width(), event->newSize().height());
        d->rightBorder->setPos(event->newSize().width() - d->rightBorder->size().width(), 0);
    }

    QGraphicsWidget::resizeEvent(event);
}

void ScrollWidget::keyPressEvent(QKeyEvent *event)
{
    d->handleKeyPressEvent(event);
}

void ScrollWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->widget) {
        return;
    }

    d->handleMouseMoveEvent(event);
    event->accept();

    return QGraphicsWidget::mouseMoveEvent(event);
}

void ScrollWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->widget) {
        return;
    }

    d->handleMousePressEvent(event);

    if (event->button() == Qt::LeftButton) {
        event->accept();
    } else {
        QGraphicsWidget::mousePressEvent(event);
    }
}

void ScrollWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->widget) {
        return;
    }

    d->handleMouseReleaseEvent(event);
    event->accept();
}

void ScrollWidget::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (!d->widget) {
        return;
    }
    d->handleWheelEvent(event);
    event->accept();
}

bool ScrollWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (!d->widget) {
        return false;
    }

    if (watched == d->scrollingWidget && (event->type() == QEvent::GraphicsSceneResize ||
         event->type() == QEvent::Move)) {
        emit viewportGeometryChanged(viewportGeometry());
    } else if (watched == d->widget.data() && event->type() == QEvent::GraphicsSceneResize) {
        d->stopAnimations();
        d->adjustScrollbarsTimer->start(200);
        updateGeometry();
        ensureItemVisible(d->widget.data());
    } else if (watched == d->widget.data() && event->type() == QEvent::GraphicsSceneMove) {
        d->horizontalScrollBar->blockSignals(true);
        d->verticalScrollBar->blockSignals(true);
        d->horizontalScrollBar->setValue(-d->widget.data()->pos().x()/10);
        d->verticalScrollBar->setValue(-d->widget.data()->pos().y()/10);
        d->horizontalScrollBar->blockSignals(false);
        d->verticalScrollBar->blockSignals(false);
    }

    return false;
}

QSizeF ScrollWidget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    if (!d->widget || which == Qt::MaximumSize) {
        return QGraphicsWidget::sizeHint(which, constraint);
    //FIXME: it should ake the minimum hint of the contained widget, but the result is in a ridiculously big widget
    } else if (which == Qt::MinimumSize) {
        return QSizeF(KIconLoader::SizeEnormous, KIconLoader::SizeEnormous);
    }

    QSizeF hint = d->widget.data()->effectiveSizeHint(which, constraint)+QSize(d->borderSize*2, d->borderSize*2);
    if (d->horizontalScrollBar && d->horizontalScrollBar->isVisible()) {
        hint += QSize(0, d->horizontalScrollBar->size().height());
    }
    if (d->verticalScrollBar && d->verticalScrollBar->isVisible()) {
        hint += QSize(d->verticalScrollBar->size().width(), 0);
    }

    return hint;
}


bool ScrollWidget::sceneEventFilter(QGraphicsItem *i, QEvent *e)
{
    //only the scrolling widget and its children
    if (!d->widget.data() ||
        (!d->scrollingWidget->isAncestorOf(i) && i != d->scrollingWidget) ||
        i == d->horizontalScrollBar || i == d->verticalScrollBar) {
        return false;
    }

    bool stealThisEvent = d->stealEvent;
    //still pass around mouse moves: try to make still possible to make items start a drag event. thi could be either necessary or annoying, let's see how it goes. (add QEvent::GraphicsSceneMouseMove to block them)
    stealThisEvent &= (e->type() == QEvent::GraphicsSceneMousePress ||
                       e->type() == QEvent::GraphicsSceneMouseRelease);
#if DEBUG
    qDebug()<<"sceneEventFilter = " <<i<<", "
            <<QTime::currentTime().toString(QString::fromLatin1("hh:mm:ss.zzz"));
#endif
    switch (e->type()) {
    case QEvent::GraphicsSceneMousePress:
        d->handleMousePressEvent(static_cast<QGraphicsSceneMouseEvent*>(e));
        break;
    case QEvent::GraphicsSceneMouseMove:
        d->handleMouseMoveEvent(static_cast<QGraphicsSceneMouseEvent*>(e));
        break;
    case QEvent::GraphicsSceneMouseRelease:
        d->handleMouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(e));
        break;
    case QEvent::GraphicsSceneWheel:
        d->handleWheelEvent(static_cast<QGraphicsSceneWheelEvent*>(e));
    //Multitouch related events
    case QEvent::TouchBegin:
        d->handleMousePressEvent(static_cast<QGraphicsSceneMouseEvent*>(e));
        break;
    case QEvent::TouchUpdate: {
        QList<QTouchEvent::TouchPoint> touchPoints = static_cast<QTouchEvent *>(e)->touchPoints();
        if (touchPoints.count() == 2) {
            const QTouchEvent::TouchPoint &touchPoint0 = touchPoints.first();
            const QTouchEvent::TouchPoint &touchPoint1 = touchPoints.last();
            const QLineF line0(touchPoint0.lastPos(), touchPoint1.lastPos());
            const QLineF line1(touchPoint0.pos(), touchPoint1.pos());
            const QLineF startLine(touchPoint0.startPos(), touchPoint1.startPos());
            const QPointF point = line1.pointAt(0.5);
            const QPointF lastPoint = line0.pointAt(0.5);

            if (d->multitouchGesture == ScrollWidgetPrivate::GestureNone) {
                d->multitouchGesture = ScrollWidgetPrivate::GestureUndefined;
            }
            if (d->multitouchGesture == ScrollWidgetPrivate::GestureUndefined) {
                const int zoomDistance = qAbs(line1.length() - startLine.length());
                const int dragDistance = (startLine.pointAt(0.5) - point).manhattanLength();

                if (zoomDistance - dragDistance > 30) {
                    d->multitouchGesture = ScrollWidgetPrivate::GestureZoom;
                } else if (dragDistance - zoomDistance > 30) {
                    d->multitouchGesture = ScrollWidgetPrivate::GestureScroll;
                }
            }

            if (d->multitouchGesture ==  ScrollWidgetPrivate::GestureScroll) {
                QGraphicsSceneMouseEvent fakeEvent;
                fakeEvent.setPos(point);
                fakeEvent.setLastPos(lastPoint);
                d->handleMouseMoveEvent(&fakeEvent);
            } else if (d->multitouchGesture == ScrollWidgetPrivate::GestureZoom) {
                if (d->widget && d->widget.data()->property("zoomFactor").isValid()) {
                    qreal scaleFactor = 1;
                    if (line0.length() > 0) {
                        scaleFactor = line1.length() / line0.length();
                    }

                    qreal zoom = d->widget.data()->property("zoomFactor").toReal();
                    d->widget.data()->setProperty("zoomFactor", zoom * scaleFactor);
                }
            }
        }
        break;
    }
    case QEvent::TouchEnd:
        d->handleMouseReleaseEvent(static_cast<QGraphicsSceneMouseEvent*>(e));
        break;
    default:
        break;
    }
    if (stealThisEvent)
        return true;
    return QGraphicsWidget::sceneEventFilter(i, e);
}

void Plasma::ScrollWidget::setAlignment(Qt::Alignment align)
{
    d->alignment = align;
    if (d->widget.data() &&
        d->widget.data()->isVisible()) {
        d->widget.data()->setPos(d->minXExtent(),
                                 d->minYExtent());
    }
}

Qt::Alignment Plasma::ScrollWidget::alignment() const
{
    return d->alignment;
}

void ScrollWidget::setOverShoot(bool enable)
{
    d->hasOvershoot = enable;
}

bool ScrollWidget::hasOverShoot() const
{
    return d->hasOvershoot;
}

} // namespace Plasma


#include <scrollwidget.moc>

