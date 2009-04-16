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
#include <QTimer>
#include <QtGui/QVBoxLayout>
#include <QtGui/QGraphicsSceneEvent>
#include <QtGui/QGraphicsView>
#include <QtGui/QGraphicsWidget>
#include <QApplication>
#include <QDesktopWidget>

#include <kdebug.h>
#include <kwindowsystem.h>
#include <netwm.h>

#include "plasma/applet.h"
#include "plasma/animator.h"
#include "plasma/extender.h"
#include "plasma/private/extender_p.h"
#include "plasma/framesvg.h"
#include "plasma/theme.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

namespace Plasma
{

class DialogPrivate
{
public:
    DialogPrivate(Dialog *dialog)
            : q(dialog),
              background(0),
              view(0),
              graphicsWidget(0),
              resizeCorners(Dialog::NoCorner),
              resizeStartCorner(Dialog::NoCorner),
              moveTimer(0),
              hideAnimId(0)
    {
    }

    ~DialogPrivate()
    {
    }

    void themeUpdated();
    void adjustView();
    void updateResizeCorners();
    void progressHide(qreal amount);

    Plasma::Dialog *q;

    /**
     * Holds the background SVG, to be re-rendered when the cache is invalidated,
     * for example by resizing the dialogue.
     */
    Plasma::FrameSvg *background;
    QGraphicsView *view;
    QGraphicsWidget *graphicsWidget;
    Dialog::ResizeCorners resizeCorners;
    QMap<Dialog::ResizeCorner, QRect> resizeAreas;
    Dialog::ResizeCorner resizeStartCorner;
    QTimer *moveTimer;
    Direction hideDirection;
    int hideAnimId;
};

void DialogPrivate::themeUpdated()
{
    qreal topHeight;
    qreal leftWidth;
    qreal rightWidth;
    qreal bottomHeight;

    background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    FrameSvg::EnabledBorders borders = FrameSvg::AllBorders;

    Extender *extender = qobject_cast<Extender*>(graphicsWidget);
    if (extender) {
        switch (extender->d->applet->location()) {
        case BottomEdge:
            borders ^= FrameSvg::BottomBorder;
            leftWidth = 0;
            rightWidth = 0;
            bottomHeight = 0;
        break;

        case TopEdge:
            borders ^= FrameSvg::TopBorder;
            topHeight = 0;
            leftWidth = 0;
            rightWidth = 0;
        break;

        case LeftEdge:
            borders ^= FrameSvg::LeftBorder;
            leftWidth = 0;
            rightWidth = 0;
        break;

        case RightEdge:
            borders ^= FrameSvg::RightBorder;
            leftWidth = 0;
            rightWidth = 0;
        break;

        default:
        break;
        }
    } else {
        QDesktopWidget *desktop = QApplication::desktop();
        QRect avail = desktop->availableGeometry(desktop->screenNumber(q));
        QRect dialogGeom = q->geometry();

        if (dialogGeom.left() <= avail.left()) {
            borders ^= FrameSvg::LeftBorder;
        }
        if (dialogGeom.top() <= avail.top()) {
            borders ^= FrameSvg::TopBorder;
        }
        //FIXME: that 2 pixels offset has probably something to do with kwin
        if (dialogGeom.right() + 2 > avail.right()) {
            borders ^= FrameSvg::RightBorder;
        }
        if (dialogGeom.bottom() + 2 > avail.bottom()) {
            borders ^= FrameSvg::BottomBorder;
        }
    }

    background->setEnabledBorders(borders);

    if (!extender) {
        background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);
    }

    q->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);
    q->update();
}

void DialogPrivate::adjustView()
{
    if (view && graphicsWidget) {
        QSize prevSize = q->size();
        kDebug() << "Widget size:" << graphicsWidget->size()
                 << "| Widget size hint:" << graphicsWidget->effectiveSizeHint(Qt::PreferredSize)
                 << "| Widget minsize hint:" << graphicsWidget->minimumSize()
                 << "| Widget maxsize hint:" << graphicsWidget->maximumSize()
                 << "| Widget bounding rect:" << graphicsWidget->sceneBoundingRect();

        //set the sizehints correctly:
        int left, top, right, bottom;
        q->getContentsMargins(&left, &top, &right, &bottom);

        q->setMinimumSize(qMin(int(graphicsWidget->minimumSize().width()) + left + right, QWIDGETSIZE_MAX),
                          qMin(int(graphicsWidget->minimumSize().height()) + top + bottom, QWIDGETSIZE_MAX));
        q->setMaximumSize(qMin(int(graphicsWidget->maximumSize().width()) + left + right, QWIDGETSIZE_MAX),
                          qMin(int(graphicsWidget->maximumSize().height()) + top + bottom, QWIDGETSIZE_MAX));
        q->resize(qMin(int(graphicsWidget->size().width()) + left + right, QWIDGETSIZE_MAX),
                          qMin(int(graphicsWidget->size().height()) + top + bottom, QWIDGETSIZE_MAX));
        q->updateGeometry();

        //reposition and resize the view.
        //force a valid rect, otherwise it will take up the whole scene
        QRectF sceneRect(graphicsWidget->sceneBoundingRect());

        sceneRect.setWidth(qMax(qreal(1), sceneRect.width()));
        sceneRect.setHeight(qMax(qreal(1), sceneRect.height()));
        view->setSceneRect(sceneRect);

        view->resize(graphicsWidget->size().toSize());
        view->centerOn(graphicsWidget);

        if (q->size() != prevSize) {
            //the size of the dialog has changed, emit the signal:
            emit q->dialogResized();
        }
    }
}

Dialog::Dialog(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f),
      d(new DialogPrivate(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    setWindowFlags(Qt::FramelessWindowHint);
    d->background = new FrameSvg(this);
    d->background->setImagePath("dialogs/background");
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    d->background->resizeFrame(size());

    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);

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
    QRect target = e->rect();
    QRect source = target;

    if (d->hideAnimId) {
        target = rect();
        source = target;
        switch (d->hideDirection) {
            case Plasma::Up: {
                int bottomMargin = height() - d->view->geometry().bottom();
                target.setBottom(d->view->viewport()->geometry().bottom() - bottomMargin); 
                source.setTop(-d->view->viewport()->y() + bottomMargin);
            }
            break;

            case Plasma::Down:
                target.setX(d->view->viewport()->x() + d->view->x());
                target.setY(d->view->viewport()->y() + d->view->y());
                source.moveTo(0, 0);
            break;

            case Plasma::Right: {
                target.setLeft(d->view->viewport()->x());
                source.setRight(target.width() - 1);
            }
            break;

            case Plasma::Left: {
                target.setWidth(d->view->viewport()->geometry().right() - 1);
                source.setLeft(width() - target.width());
            }
            break;
        }
    }

    QPainter p(this);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    d->background->paintFrame(&p, target, source);
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
    } else if (!(event->buttons() & Qt::LeftButton)) {
        unsetCursor();
    }

    // here we take care of resize..
    if (d->resizeStartCorner != Dialog::NoCorner) {
        int newWidth;
        int newHeight;
        QPoint position;

        switch(d->resizeStartCorner) {
            case Dialog::NorthEast:
                newWidth = qMin(maximumWidth(), qMax(minimumWidth(), event->x()));
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height() - event->y()));
                position = QPoint(x(), y() + height() - newHeight);
                break;
            case Dialog::NorthWest:
                newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width() - event->x()));
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height() - event->y()));
                position = QPoint(x() + width() - newWidth, y() + height() - newHeight);
                break;
            case Dialog::SouthWest:
                newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width() - event->x()));
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), event->y()));
                position = QPoint(x() + width() - newWidth, y());
                break;
            case Dialog::SouthEast:
                newWidth = qMin(maximumWidth(), qMax(minimumWidth(), event->x()));
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), event->y()));
                position = QPoint(x(), y());
                break;
             default:
                newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width()));
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height()));
                position = QPoint(x(), y());
                break;
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
        unsetCursor();
        emit dialogResized();
    }

    QWidget::mouseReleaseEvent(event);
}

void Dialog::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape) {
        hide();
    }
}

bool Dialog::event(QEvent *event)
{
    if (event->type() == QEvent::Paint) {
        QPainter p(this);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(rect(), Qt::transparent);
    }

    return QWidget::event(event);
}

void Dialog::resizeEvent(QResizeEvent *e)
{
    d->background->resizeFrame(e->size());

    setMask(d->background->mask());

    if (d->resizeStartCorner != Dialog::NoCorner && d->view && d->graphicsWidget) {
        d->graphicsWidget->resize(d->view->size());

        QRectF sceneRect(d->graphicsWidget->sceneBoundingRect());
        sceneRect.setWidth(qMax(qreal(1), sceneRect.width()));
        sceneRect.setHeight(qMax(qreal(1), sceneRect.height()));
        d->view->setSceneRect(sceneRect);
        d->view->centerOn(d->graphicsWidget);
    }

    d->updateResizeCorners();
}

void DialogPrivate::updateResizeCorners()
{
    const int resizeAreaMargin = 20;
    const QRect r = q->rect();

    resizeAreas.clear();
    if (resizeCorners & Dialog::NorthEast) {
        resizeAreas[Dialog::NorthEast] = QRect(r.right() - resizeAreaMargin, 0,
                                               resizeAreaMargin, resizeAreaMargin);
    }

    if (resizeCorners & Dialog::NorthWest) {
        resizeAreas[Dialog::NorthWest] = QRect(0, 0, resizeAreaMargin, resizeAreaMargin);
    }

    if (resizeCorners & Dialog::SouthEast) {
        resizeAreas[Dialog::SouthEast] = QRect(r.right() - resizeAreaMargin,
                                               r.bottom() - resizeAreaMargin,
                                               resizeAreaMargin, resizeAreaMargin);
    }

    if (resizeCorners & Dialog::SouthWest) {
        resizeAreas[Dialog::SouthWest] = QRect(0, r.bottom() - resizeAreaMargin,
                                               resizeAreaMargin, resizeAreaMargin);
    }

}

void Dialog::setGraphicsWidget(QGraphicsWidget *widget)
{
    if (d->graphicsWidget) {
        d->graphicsWidget->removeEventFilter(this);
    }

    d->graphicsWidget = widget;

    if (widget) {
        if (!layout()) {
            QVBoxLayout *lay = new QVBoxLayout(this);
            lay->setMargin(0);
            lay->setSpacing(0);
        }

        d->themeUpdated();

        if (!d->view) {
            d->view = new QGraphicsView(this);
            d->view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            d->view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
    return d->graphicsWidget;
}

bool Dialog::eventFilter(QObject *watched, QEvent *event)
{
    if (d->resizeStartCorner == Dialog::NoCorner && watched == d->graphicsWidget &&
        (event->type() == QEvent::GraphicsSceneResize || event->type() == QEvent::GraphicsSceneMove)) {
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

    //check if the widget size is still synced with the view
    d->themeUpdated();

    if (d->graphicsWidget && d->view && d->graphicsWidget->size().toSize() != d->view->size()) {
        d->adjustView();
    }

    if (d->view) {
        d->view->setFocus();
    }

    if (d->graphicsWidget) {
        d->graphicsWidget->setFocus();
    }

    emit dialogVisible(true);
}

void Dialog::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event)

    if (!d->moveTimer) {
        d->moveTimer = new QTimer(this);
        d->moveTimer->setSingleShot(true);
        connect(d->moveTimer, SIGNAL(timeout()), this, SLOT(themeUpdated()));
    }
    d->moveTimer->start(200);
}

void Dialog::setResizeHandleCorners(ResizeCorners corners)
{
    if (d->resizeCorners != corners) {
        d->resizeCorners = corners;
        d->updateResizeCorners();
    }
}

Dialog::ResizeCorners Dialog::resizeCorners() const
{
    return d->resizeCorners;
}

void Dialog::animatedHide(Plasma::Direction direction)
{
    if (d->hideAnimId) {
        // already hiding
        return;
    }

    if (KWindowSystem::compositingActive() && d->view) {
        //TODO: implement for the QWidget scenario too
        d->hideDirection = direction;
        d->hideAnimId = Animator::self()->customAnimation(20, 200, Animator::EaseOutCurve,
                                                          this, "progressHide");
    } else {
        hide();
    }
}

void DialogPrivate::progressHide(qreal amount)
{
    //kDebug() << amount;
    if (qFuzzyCompare(amount, 1.0)) {
        q->hide();
        view->viewport()->move(0, 0);
        hideAnimId = 0;
        return;
    }

    int xtrans = 0;
    int ytrans = 0;

    //TODO: switch between directions
    switch (hideDirection) {
        case Plasma::Up:
            ytrans = -(amount * view->height());
        break;

        case Plasma::Down:
            ytrans = amount * view->height();
        break;

        case Plasma::Right:
            xtrans = amount * view->width();
        break;

        case Plasma::Left:
            xtrans = -(amount * view->width());
        break;
    }

    view->viewport()->move(xtrans, ytrans);
    q->update();
}

bool Dialog::inControlArea(const QPoint &point)
{
    foreach (const QRect &r, d->resizeAreas) {
        if (r.contains(point)) {
            return true;
        }
    }
    return false;
}

}
#include "dialog.moc"
