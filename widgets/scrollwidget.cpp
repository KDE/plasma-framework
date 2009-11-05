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
#include <QApplication>
#include <QWidget>
#include <QTimer>

//KDE
#include <kmimetype.h>
#include <kdebug.h>

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
          widget(0),
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

    void adjustScrollbars()
    {
        verticalScrollBar->nativeWidget()->setMaximum(qMax(0, int((widget->size().height() - scrollingWidget->size().height())/10)));

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

        horizontalScrollBar->nativeWidget()->setMaximum(qMax(0, int((widget->size().width() - scrollingWidget->size().width())/10)));

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

         if (widget && !topBorder && widget->size().height() > q->size().height()) {
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
        } else if (topBorder && widget && widget->size().height() <= q->size().height()) {
            //FIXME: in some cases topBorder->deleteLater() is deleteNever(), why?
            topBorder->hide();
            bottomBorder->hide();
            topBorder->deleteLater();
            bottomBorder->deleteLater();
            topBorder = 0;
            bottomBorder = 0;
        }


        if (widget && !leftBorder && widget->size().width() > q->size().width()) {
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
        } else if (leftBorder && widget && widget->size().width() <= q->size().width()) {
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

        QSizeF widgetSize = widget->size();
        if (widget->sizePolicy().expandingDirections() & Qt::Horizontal) {
            //keep a 1 pixel border
            widgetSize.setWidth(scrollingWidget->size().width()-4);
        }
        if (widget->sizePolicy().expandingDirections() & Qt::Vertical) {
            widgetSize.setHeight(scrollingWidget->size().height()-4);
        }
        widget->resize(widgetSize);
    }

    void verticalScroll(int value)
    {
        if (!dragging) {
            widget->setPos(QPoint(widget->pos().x(), -value*10));
        }
    }

    void horizontalScroll(int value)
    {
        if (!dragging) {
            widget->setPos(QPoint(-value*10, widget->pos().y()));
        }
    }

    void adjustClipping()
    {
         const bool clip = widget->size().width() > scrollingWidget->size().width() || widget->size().height() > scrollingWidget->size().height();

         scrollingWidget->setFlag(QGraphicsItem::ItemClipsChildrenToShape, clip);
    }

    void makeRectVisible()
    {
        QRectF viewRect = scrollingWidget->boundingRect();
        //ensure the rect is not outside the widget bounding rect
        QRectF mappedRect = QRectF(QPointF(qBound((qreal)0.0, rectToBeVisible.x(), widget->size().width() - rectToBeVisible.width()),
                                           qBound((qreal)0.0, rectToBeVisible.y(), widget->size().height() - rectToBeVisible.height())),
                                           rectToBeVisible.size());
        mappedRect = widget->mapToItem(scrollingWidget, mappedRect).boundingRect();

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
        widget, Plasma::Animator::SlideOutMovement,
                                                (widget->pos() + delta).toPoint());
    }

    ScrollWidget *q;
    QGraphicsWidget *scrollingWidget;
    QGraphicsWidget *widget;
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
    QRectF rectToBeVisible;
    bool dragging;
    int animId;
};


ScrollWidget::ScrollWidget(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new ScrollWidgetPrivate(this))
{
    d->layout = new QGraphicsGridLayout(this);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->layout->setContentsMargins(1, 1, 1, 1);
    d->scrollingWidget = new QGraphicsWidget(this);
    d->layout->addItem(d->scrollingWidget, 0, 0);
    d->borderSvg = new Plasma::Svg(this);
    d->borderSvg->setImagePath("widgets/scrollwidget");

    d->verticalScrollBarPolicy = Qt::ScrollBarAsNeeded;
    d->verticalScrollBar = new Plasma::ScrollBar(this);
    d->layout->addItem(d->verticalScrollBar, 0, 1);
    d->verticalScrollBar->nativeWidget()->setMinimum(0);
    d->verticalScrollBar->nativeWidget()->setMaximum(100);
    connect(d->verticalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(verticalScroll(int)));

    d->horizontalScrollBarPolicy = Qt::ScrollBarAsNeeded;
    d->horizontalScrollBar = new Plasma::ScrollBar(this);
    d->horizontalScrollBar->setOrientation(Qt::Horizontal);
    d->layout->addItem(d->horizontalScrollBar, 1, 0);
    d->horizontalScrollBar->nativeWidget()->setMinimum(0);
    d->horizontalScrollBar->nativeWidget()->setMaximum(100);
    connect(d->horizontalScrollBar, SIGNAL(valueChanged(int)), this, SLOT(horizontalScroll(int)));
}

ScrollWidget::~ScrollWidget()
{
    delete d;
}

void ScrollWidget::setWidget(QGraphicsWidget *widget)
{
    if (d->widget && d->widget != widget) {
        d->widget->removeEventFilter(this);
        delete d->widget;
    }

    d->widget = widget;
    Plasma::Animator::self()->registerScrollingManager(this);
    //it's not good it's setting a size policy here, but it's done to be retrocompatible with older applications
    widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    widget->setParentItem(d->scrollingWidget);
    widget->setPos(QPoint(0,0));
    widget->installEventFilter(this);
    d->adjustScrollbars();
}

QGraphicsWidget *ScrollWidget::widget() const
{
    return d->widget;
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
    while (parentOfItem != d->widget) {
        if (!parentOfItem) {
            return;
        }

        parentOfItem = parentOfItem->parentItem();
    }

    QRectF rect(d->widget->mapFromScene(item->scenePos()), item->boundingRect().size());
    d->rectToBeVisible = rect;

    // We need to wait for the parent item to resize...
    QTimer::singleShot(0, this, SLOT(makeRectVisible()));
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
    return d->widget->size();
}

void ScrollWidget::setScrollPosition(const QPointF &position)
{
    d->widget->setPos(-position);
}

QPointF ScrollWidget::scrollPosition() const
{
    return -d->widget->pos();
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
        d->widget->setFocus();
    }
}


void ScrollWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    if (!d->widget) {
        return;
    }

    d->adjustScrollbars();

    d->adjustClipping();

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
    if (d->animId) {
        Animator::self()->stopItemMovement(d->animId);
    }
}

void ScrollWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    event->ignore();
}

void ScrollWidget::wheelEvent(QGraphicsSceneWheelEvent *event)
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

    if (watched == d->widget && event->type() == QEvent::GraphicsSceneResize) {
        d->adjustScrollbars();
        d->adjustClipping();
        //force to refresh the size hint
        layout()->invalidate();
    } else if (watched == d->widget && event->type() == QEvent::GraphicsSceneMove) {
        d->horizontalScrollBar->blockSignals(true);
        d->verticalScrollBar->blockSignals(true);
        d->horizontalScrollBar->setValue(-d->widget->pos().x()/10);
        d->verticalScrollBar->setValue(-d->widget->pos().y()/10);
        d->horizontalScrollBar->blockSignals(false);
        d->verticalScrollBar->blockSignals(false);
    }

    return false;
}

QSizeF ScrollWidget::sizeHint(Qt::SizeHint which, const QSizeF & constraint) const
{
    QSizeF hint = QGraphicsWidget::sizeHint(which, constraint);

    if (which == Qt::PreferredSize && d->widget) {
        return (d->widget->size()+QSize(4,4)).expandedTo(d->widget->effectiveSizeHint(Qt::PreferredSize));
    }

    return hint;
}

} // namespace Plasma

#include <scrollwidget.moc>

