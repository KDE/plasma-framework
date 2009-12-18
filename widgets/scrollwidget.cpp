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
//Qt
#include <QGraphicsSceneResizeEvent>
#include <QGraphicsGridLayout>
#include <QGraphicsScene>
#include <QApplication>
#include <QWidget>
#include <QTimer>

//KDE
#include <kmimetype.h>
#include <kdebug.h>
#include <kglobalsettings.h>

//Plasma
#include <plasma/widgets/scrollbar.h>
#include <plasma/widgets/svgwidget.h>
#include <plasma/animator.h>
#include <plasma/svg.h>

namespace Plasma
{

class ScrollWidgetPrivate
{
public:
    ScrollWidgetPrivate(ScrollWidget *parent)
        : q(parent),
          topBorder(0),
          bottomBorder(0),
          leftBorder(0),
          rightBorder(0),
          dragging(false),
          animId(0)
    {
    }

    ~ScrollWidgetPrivate()
    {
    }

    void commonConstructor()
    {
        q->setFocusPolicy(Qt::StrongFocus);
        layout = new QGraphicsGridLayout(q);
        q->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        layout->setContentsMargins(1, 1, 1, 1);
        scrollingWidget = new QGraphicsWidget(q);
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
    }

    void adjustScrollbars()
    {
        if (!widget) {
            return;
        }

        verticalScrollBar->nativeWidget()->setMaximum(qMax(0, int((widget.data()->size().height() - scrollingWidget->size().height())/10)));

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

        animId = Animator::self()->moveItem(
        widget.data(), Plasma::Animator::SlideOutMovement,
                                                (widget.data()->pos() + delta).toPoint());
    }

    void makeItemVisible()
    {
        if (!widget) {
            return;
        }

        QRectF rect(widget.data()->mapFromScene(itemToBeVisible->scenePos()), itemToBeVisible->boundingRect().size());
        rectToBeVisible = rect;

        makeRectVisible();
    }

    void cleanupDragHandles(QObject *destroyed)
    {
        dragHandles.remove(static_cast<QGraphicsWidget *>(destroyed));
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
    QGraphicsItem *itemToBeVisible;
    QRectF rectToBeVisible;
    QPointF dragHandleClicked;
    QSet<QGraphicsWidget *>dragHandles;
    bool dragging;
    int animId;
    QTimer *adjustScrollbarsTimer;
    static const int borderSize = 4;
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
        d->widget.data()->removeEventFilter(this);
        delete d->widget.data();
    }

    d->widget = widget;
    Plasma::Animator::self()->registerScrollingManager(this);
    //it's not good it's setting a size policy here, but it's done to be retrocompatible with older applications

    if (widget) {
        widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        widget->setParentItem(d->scrollingWidget);
        widget->setPos(QPoint(0,0));
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

    d->itemToBeVisible = item;

    // We need to wait for the parent item to resize...
    QTimer::singleShot(0, this, SLOT(makeItemVisible()));
}

void ScrollWidget::registerAsDragHandle(QGraphicsWidget *item)
{
    if (!d->widget || !item || d->dragHandles.contains(item)) {
        return;
    }

    QGraphicsItem *parentOfItem = item->parentItem();
    while (parentOfItem != d->widget.data()) {
        if (!parentOfItem) {
            return;
        }

        parentOfItem = parentOfItem->parentItem();
    }

    connect(item, SIGNAL(destroyed(QObject *)), this, SLOT(cleanupDragHandles(QObject *)));
    item->installEventFilter(this);
    d->dragHandles.insert(item);
}

void ScrollWidget::unregisterAsDragHandle(QGraphicsWidget *item)
{
    if (item) {
        item->removeEventFilter(this);
        d->dragHandles.remove(item);
    }
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
    return d->widget ? d->widget.data()->size() : QSizeF();
}

void ScrollWidget::setScrollPosition(const QPointF &position)
{
    if (d->widget) {
        d->widget.data()->setPos(-position.toPoint());
    }
}

QPointF ScrollWidget::scrollPosition() const
{
    return d->widget ? -d->widget.data()->pos() : QPointF();
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

void ScrollWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->widget) {
        return;
    }

    if (d->animId) {
        Animator::self()->stopItemMovement(d->animId);
    }

    event->ignore();
}

void ScrollWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    if (!(event->buttons() & Qt::LeftButton)) {
        event->ignore();
    }

    if (d->animId) {
        Animator::self()->stopItemMovement(d->animId);
    }
}

void ScrollWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

void ScrollWidget::wheelEvent(QGraphicsSceneWheelEvent *)
{
    if (d->animId) {
        Animator::self()->stopItemMovement(d->animId);
    }

}

bool ScrollWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (!d->widget) {
        return false;
    }

    if (watched == d->widget.data() && event->type() == QEvent::GraphicsSceneResize) {
        d->adjustScrollbarsTimer->start(200);
    } else if (watched == d->widget.data() && event->type() == QEvent::GraphicsSceneMove) {
        d->horizontalScrollBar->blockSignals(true);
        d->verticalScrollBar->blockSignals(true);
        d->horizontalScrollBar->setValue(-d->widget.data()->pos().x()/10);
        d->verticalScrollBar->setValue(-d->widget.data()->pos().y()/10);
        d->horizontalScrollBar->blockSignals(false);
        d->verticalScrollBar->blockSignals(false);
    } else if (d->dragHandles.contains(static_cast<QGraphicsWidget *>(watched))) {
        if (event->type() == QEvent::GraphicsSceneMousePress ||
            event->type() == QEvent::GraphicsSceneMouseMove ||
            event->type() == QEvent::GraphicsSceneMouseRelease) {
            if (scene()) {
                scene()->sendEvent(this, event);
            }

            QGraphicsSceneMouseEvent *me = static_cast<QGraphicsSceneMouseEvent *>(event);
            if (event->type() == QEvent::GraphicsSceneMousePress) {
                d->dragHandleClicked = me->scenePos();
            } else if (event->type() == QEvent::GraphicsSceneMouseRelease &&
                       (d->dragHandleClicked.toPoint() - me->scenePos().toPoint()).manhattanLength() > KGlobalSettings::dndEventDelay()) {
                return true;
            }
        }
    }

    return false;
}

QSizeF ScrollWidget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF hint = QGraphicsWidget::sizeHint(which, constraint);

    if (which == Qt::PreferredSize && d->widget) {
        return (d->widget.data()->size()+QSize(d->borderSize, d->borderSize)).expandedTo(d->widget.data()->effectiveSizeHint(Qt::PreferredSize));
    }

    return hint;
}

} // namespace Plasma

#include <scrollwidget.moc>

