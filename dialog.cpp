/*
 * Copyright 2008 by Alessandro Diaferia <alediaferia@gmail.com>
 * Copyright 2007 by Alexis Ménard <darktears31@gmail.com>
 * Copyright 2007 Sebastian Kuegler <sebas@kde.org>
 * Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#include "dialog.h"

#include <QPainter>
#include <QSvgRenderer>
#include <QResizeEvent>
#include <QMouseEvent>
#ifdef Q_WS_X11
#include <QX11Info>
#endif
#include <QBitmap>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGraphicsSceneEvent>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsWidget>

#include <KDebug>
#include <NETRootInfo>

#include <plasma/panelsvg.h>
#include <plasma/theme.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

const int resizeAreaMargin = 20;

namespace Plasma
{

class DialogPrivate
{
public:
    DialogPrivate(Dialog *dialog)
            : q(dialog),
              background(0),
              view(0),
              widget(0),
              resizeCorners(Dialog::NoCorner)
    {
    }

    ~DialogPrivate()
    {
    }

    void themeUpdated();
    void adjustView();

    Plasma::Dialog *q;
    /**
     * Holds the background SVG, to be re-rendered when the cache is invalidated,
     * for example by resizing the dialogue.
     */
    Plasma::PanelSvg *background;
    QGraphicsView *view;
    QGraphicsWidget *widget;
    Dialog::ResizeCorners resizeCorners;
    QMap<Dialog::ResizeCorner, QRect> resizeAreas;
    Dialog::ResizeCorner resizeStartCorner;
};

void DialogPrivate::themeUpdated()
{
    const int topHeight = background->marginSize(Plasma::TopMargin);
    const int leftWidth = background->marginSize(Plasma::LeftMargin);
    const int rightWidth = background->marginSize(Plasma::RightMargin);
    const int bottomHeight = background->marginSize(Plasma::BottomMargin);
    q->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
}

void DialogPrivate::adjustView()
{
    if (view && widget) {
        QSize prevSize = q->size();

        kDebug() << "Widget size:" << widget->size()
                 << "| Widget size hint:" << widget->effectiveSizeHint(Qt::PreferredSize)
                 << "| Widget bounding rect:" << widget->boundingRect();

        QRectF boundingRect = widget->boundingRect();
        boundingRect.setSize(widget->effectiveSizeHint(Qt::PreferredSize));

        //reposition and resize the view.
        view->setSceneRect(widget->mapToScene(boundingRect).boundingRect());
        view->resize(view->mapFromScene(view->sceneRect()).boundingRect().size());
        view->centerOn(widget);

        //set the sizehints correctly:
        int left, top, right, bottom;
        q->getContentsMargins(&left, &top, &right, &bottom);
        q->setMinimumSize(view->size().width() + left + right,
                          view->size().height() + top + bottom);
        q->setMaximumSize(view->size().width() + left + right,
                          view->size().height() + top + bottom);
        q->updateGeometry();

        if (q->size() != prevSize) {
            //the size of the dialog has changed, emit the signal:
            emit q->dialogResized();
        }
    }
}

Dialog::Dialog( QWidget * parent, Qt::WindowFlags f )
    : QWidget(parent, f),
      d(new DialogPrivate(this))
{
    d->background = new PanelSvg(this);
    d->background->setImagePath("dialogs/background");
    d->background->setEnabledBorders(PanelSvg::AllBorders);
    d->background->resizePanel(size());

    connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
    d->themeUpdated();

    setMouseTracking(true);
}

Dialog::~Dialog()
{
    delete d;
}

void Dialog::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    p.setClipRect(e->rect());
    p.setCompositionMode(QPainter::CompositionMode_Source );
    p.fillRect(rect(), Qt::transparent);
    d->background->paintPanel(&p);

    //we set the resize handlers
    d->resizeAreas.clear();
    d->resizeAreas[Dialog::NorthEast] = QRect(rect().right() - resizeAreaMargin, 0, 
                                         resizeAreaMargin, resizeAreaMargin);

    d->resizeAreas[Dialog::NorthWest] = QRect(0,0, resizeAreaMargin, resizeAreaMargin);

    d->resizeAreas[Dialog::SouthEast] = QRect(rect().right() - resizeAreaMargin,
                                        rect().bottom() - resizeAreaMargin,
                                        resizeAreaMargin, resizeAreaMargin);

    d->resizeAreas[Dialog::SouthWest] = QRect(0, rect().bottom() - resizeAreaMargin,
                                        resizeAreaMargin, resizeAreaMargin);
}

void Dialog::mouseMoveEvent(QMouseEvent *event)
{
    if (d->resizeAreas[Dialog::NorthEast].contains(event->pos()) && d->resizeCorners & Dialog::NorthEast) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (d->resizeAreas[Dialog::NorthWest].contains(event->pos()) && d->resizeCorners & Dialog::NorthWest) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (d->resizeAreas[Dialog::SouthEast].contains(event->pos()) && d->resizeCorners & Dialog::SouthEast) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (d->resizeAreas[Dialog::SouthWest].contains(event->pos()) && d->resizeCorners & Dialog::SouthWest) {
        setCursor(Qt::SizeBDiagCursor);
    } else {
        unsetCursor();
    }

    // here we take care of resize..
    if (d->resizeStartCorner != Dialog::NoCorner ) {
        int newWidth;
        int newHeight;
        QPoint position;

        switch(d->resizeStartCorner) {
            case Dialog::NorthEast:
                newWidth = event->x();
                newHeight = height() - event->y();
                position = QPoint(x(), y() + height() - newHeight);
                break;
            case Dialog::NorthWest:
                newWidth = width() - event->x();
                newHeight = height() - event->y();
                position = QPoint(x() + width() - newWidth, y() + height() - newHeight);
                break;
            case Dialog::SouthWest:
                newWidth = width() - event->x();
                newHeight = event->y();
                position = QPoint(x() + width() - newWidth, y());
                break;
            case Dialog::SouthEast:
                newWidth = event->x();
                newHeight = event->y();
                position = QPoint(x(), y());
                break;
             default:
                newWidth = width();
                newHeight = height();
                position = QPoint(x(), y());
                break;
        }

        // let's check for limitations
        if (newWidth < minimumWidth() || newWidth > maximumWidth()) {
            newWidth = width();
            position.setX(x());
        }

        if (newHeight < minimumHeight() || newHeight > maximumHeight()) {
            newHeight = height();
            position.setY(y());
        }

        setGeometry(QRect(position, QSize(newWidth, newHeight)));
    }

    QWidget::mouseMoveEvent(event);
}

void Dialog::mousePressEvent(QMouseEvent *event)
{
    if (d->resizeAreas[Dialog::NorthEast].contains(event->pos()) && d->resizeCorners & Dialog::NorthEast) {
        d->resizeStartCorner = Dialog::NorthEast;

    } else if (d->resizeAreas[Dialog::NorthWest].contains(event->pos()) && d->resizeCorners & Dialog::NorthWest) {
        d->resizeStartCorner = Dialog::NorthWest;

    } else if (d->resizeAreas[Dialog::SouthEast].contains(event->pos()) && d->resizeCorners & Dialog::SouthEast) {
        d->resizeStartCorner = Dialog::SouthEast;

    } else if (d->resizeAreas[Dialog::SouthWest].contains(event->pos()) && d->resizeCorners & Dialog::SouthWest) {
        d->resizeStartCorner = Dialog::SouthWest;

    } else {
        d->resizeStartCorner = Dialog::NoCorner;
    }

    QWidget::mousePressEvent(event);
}

void Dialog::mouseReleaseEvent(QMouseEvent *event)
{
    if (d->resizeStartCorner != Dialog::NoCorner) {
        d->resizeStartCorner = Dialog::NoCorner;
        emit dialogResized();
    }

    QWidget::mouseReleaseEvent(event);
}

void Dialog::resizeEvent(QResizeEvent *e)
{
    d->background->resizePanel(e->size());

    setMask(d->background->mask());
}

void Dialog::setGraphicsWidget(QGraphicsWidget *widget)
{
    if (d->widget) {
        d->widget->removeEventFilter(this);
    }

    d->widget = widget;

    if (widget) {
        if (!layout()) {
            QVBoxLayout *lay = new QVBoxLayout(this);
            lay->setMargin(0);
            lay->setSpacing(0);
        }

        if (!d->view) {
            d->view = new QGraphicsView(this);
            d->view->setFrameShape(QFrame::NoFrame);
            d->view->viewport()->setAutoFillBackground(false);
            layout()->addWidget(d->view);
        }

        d->view->setScene(widget->scene());
        d->adjustView();

        adjustSize();

        widget->installEventFilter(this);
    } else {
        delete d->view;
        d->view = 0;
    }
}

QGraphicsWidget *Dialog::graphicsWidget()
{
    return d->widget;
}

bool Dialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == d->widget && (event->type() == QEvent::GraphicsSceneResize ||
                                 event->type() == QEvent::GraphicsSceneMove)) {
        d->adjustView();
    }

    return QWidget::eventFilter(watched, event);
}

void Dialog::hideEvent(QHideEvent * event)
{
    Q_UNUSED(event);
    emit dialogVisible(false);
}

void Dialog::showEvent(QShowEvent * event)
{
    Q_UNUSED(event);
    emit dialogVisible(true);
}

void Dialog::setResizeHandleCorners(ResizeCorners corners)
{
    d->resizeCorners = corners;
    update();
}

Dialog::ResizeCorners Dialog::resizeCorners() const
{
    return d->resizeCorners;
}

}
#include "dialog.moc"
