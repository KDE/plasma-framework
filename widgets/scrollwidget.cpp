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
#include <plasma/widgets/svgwidget.h>
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
            topBorder->deleteLater();
            bottomBorder->deleteLater();
            topBorder = 0;
            bottomBorder = 0;
        } else if (!verticalScrollBar->isVisible()) {
            layout->addItem(verticalScrollBar, 0, 1);
            verticalScrollBar->show();
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
            leftBorder->deleteLater();
            rightBorder->deleteLater();
            leftBorder = 0;
            rightBorder = 0;
        } else if (!horizontalScrollBar->isVisible()) {
            layout->addItem(horizontalScrollBar, 1, 0);
            horizontalScrollBar->show();
            leftBorder = new Plasma::SvgWidget(q);
            leftBorder->setSvg(borderSvg);
            leftBorder->setElementID("border-top");
            leftBorder->setZValue(900);
            leftBorder->resize(leftBorder->effectiveSizeHint(Qt::PreferredSize));
            leftBorder->show();
            rightBorder = new Plasma::SvgWidget(q);
            rightBorder->setSvg(borderSvg);
            rightBorder->setElementID("border-bottom");
            rightBorder->setZValue(900);
            rightBorder->resize(rightBorder->effectiveSizeHint(Qt::PreferredSize));
            rightBorder->show();
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

        widget->resize(scrollingWidget->size().width(), widget->size().height());
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
    bool dragging;
};


ScrollWidget::ScrollWidget(QGraphicsWidget *parent)
    : QGraphicsWidget(parent),
      d(new ScrollWidgetPrivate(this))
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->layout = new QGraphicsGridLayout(this);
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

    QPointF deltaPos = event->pos() - event->lastPos();

    d->widget->setPos(qBound(qMin((qreal)0,-d->widget->size().width()+d->scrollingWidget->size().width()), d->widget->pos().x()+deltaPos.x(), (qreal)0),
                      qBound(qMin((qreal)0,-d->widget->size().height()+d->scrollingWidget->size().height()), d->widget->pos().y()+deltaPos.y(), (qreal)0));

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

} // namespace Plasma

#include <scrollwidget.moc>

