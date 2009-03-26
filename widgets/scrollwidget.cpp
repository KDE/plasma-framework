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

//KDE
#include <kmimetype.h>
#include <kdebug.h>

//Plasma
#include <plasma/widgets/scrollbar.h>

namespace Plasma
{

class ScrollWidgetPrivate
{
public:
    ScrollWidgetPrivate(ScrollWidget *parent)
        : q(parent),
          widget(0),
          dragging(false)
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
            if (layout->itemAt(2) == verticalScrollBar) {
                layout->removeAt(2);
            } else if (layout->itemAt(1) == verticalScrollBar) {
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
            if (layout->itemAt(2) == horizontalScrollBar) {
                layout->removeAt(2);
            } else if (layout->itemAt(1) == horizontalScrollBar) {
                layout->removeAt(1);
            }
            horizontalScrollBar->hide();
        } else if (!horizontalScrollBar->isVisible()) {
            layout->addItem(horizontalScrollBar, 1, 0);
            horizontalScrollBar->show();
        }
    }

    void verticalScroll(int value)
    {
        if (!dragging) {
            widget->setPos(QPoint(0, -value*10));
        }
    }

    void horizontalScroll(int value)
    {
        if (!dragging) {
            widget->setPos(QPoint(-value*10, 0));
        }
    }

    void adjustClipping()
    {
        const bool clip = widget->size().width() > scrollingWidget->size().width() ||
                          widget->size().height() > scrollingWidget->size().height();

        scrollingWidget->setFlag(QGraphicsItem::ItemClipsChildrenToShape, clip);
    }

    ScrollWidget *q;
    QGraphicsWidget *scrollingWidget;
    QGraphicsWidget *widget;
    QGraphicsGridLayout *layout;
    ScrollBar *verticalScrollBar;
    Qt::ScrollBarPolicy verticalScrollBarPolicy;
    ScrollBar *horizontalScrollBar;
    Qt::ScrollBarPolicy horizontalScrollBarPolicy;
    QString styleSheet;
    bool dragging;
};


ScrollWidget::ScrollWidget(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new ScrollWidgetPrivate(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->layout = new QGraphicsGridLayout(this);
    d->scrollingWidget = new QGraphicsWidget(this);
    d->layout->addItem(d->scrollingWidget, 0, 0);

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


void ScrollWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    if (!d->widget) {
        return;
    }

    d->widget->resize(d->scrollingWidget->size());

    d->adjustScrollbars();

    d->adjustClipping();

    QGraphicsWidget::resizeEvent(event);
}

void ScrollWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!d->widget) {
        return;
    }

    QPointF deltaPos = event->pos() - event->lastPos();

    d->widget->setPos(qBound(-d->widget->size().width()+d->scrollingWidget->size().width(), d->widget->pos().x()+deltaPos.x(), (qreal)0),
                      qBound(-d->widget->size().height()+d->scrollingWidget->size().height(), d->widget->pos().y()+deltaPos.y(), (qreal)0));

    d->dragging = true;
    d->horizontalScrollBar->setValue(-d->widget->pos().x()/10);
    d->verticalScrollBar->setValue(-d->widget->pos().y()/10);
    d->dragging = false;

    QGraphicsWidget::mouseMoveEvent(event);
}

void ScrollWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    event->accept();
}

void ScrollWidget::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    if (event->delta() < 0) {
        d->verticalScrollBar->setValue(d->verticalScrollBar->value()+10);
    } else {
        d->verticalScrollBar->setValue(d->verticalScrollBar->value()-10);
    }
    QGraphicsWidget::wheelEvent(event);
}

bool ScrollWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (!d->widget) {
        return false;
    }

    if (watched == d->widget && event->type() == QEvent::GraphicsSceneResize) {
        d->adjustScrollbars();
        d->adjustClipping();
    }

    return false;
}

} // namespace Plasma

#include <scrollwidget.moc>

