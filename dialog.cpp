/*
 * Copyright 2008 by Alessandro Diaferia <alediaferia@gmail.com>
 * Copyright 2007 by Alexis MÃ©nard <darktears31@gmail.com>
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
#include <QVarLengthArray>
#include <QGraphicsLayout>

#include <kdebug.h>
#include <kwindowsystem.h>
#include <netwm.h>

#include "plasma/applet.h"
#include "plasma/animator.h"
#include "plasma/containment.h"
#include "plasma/corona.h"
#include "plasma/extenders/extender.h"
#include "plasma/private/extender_p.h"
#include "plasma/framesvg.h"
#include "plasma/theme.h"
#include "plasma/windoweffects.h"

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
              resizeCorners(Dialog::NoCorner),
              resizeStartCorner(Dialog::NoCorner),
              moveTimer(0),
              aspectRatioMode(Plasma::IgnoreAspectRatio),
              resizeChecksWithBorderCheck(false)
    {
    }

    ~DialogPrivate()
    {
    }

    void scheduleBorderCheck(bool triggeredByResize = false);
    void themeChanged();
    void updateMask();
    void checkBorders();
    void checkBorders(bool updateMaskIfNeeded);
    void updateResizeCorners();
    int calculateWidthForHeightAndRatio(int height, qreal ratio);
    Plasma::Applet *applet();
    void delayedAdjustSize();

    Plasma::Dialog *q;

    /**
     * Holds the background SVG, to be re-rendered when the cache is invalidated,
     * for example by resizing the dialogue.
     */
    Plasma::FrameSvg *background;
    QGraphicsView *view;
    QWeakPointer<QGraphicsWidget> graphicsWidgetPtr;
    Dialog::ResizeCorners resizeCorners;
    QMap<Dialog::ResizeCorner, QRect> resizeAreas;
    int resizeStartCorner;
    QTimer *moveTimer;
    QTimer *adjustViewTimer;
    QTimer *adjustSizeTimer;
    QSize oldGraphicsWidgetMinimumSize;
    QSize oldGraphicsWidgetMaximumSize;
    Plasma::AspectRatioMode aspectRatioMode;
    bool resizeChecksWithBorderCheck;
};

void DialogPrivate::scheduleBorderCheck(bool triggeredByResize)
{
    //kDebug();
    if (triggeredByResize) {
        resizeChecksWithBorderCheck = true;

        // to keep the UI as fluid as possible, we call checkBorders
        // immediately when there is a resize, and therefore stop any
        // move-triggered scheduled calls to it. this keeps things
        // looking reasonable during resize while avoiding as many
        // calls to checkBorders as possible
        if (moveTimer) {
            moveTimer->stop();
        }

        checkBorders();
        return;
    }

    if (!moveTimer) {
        moveTimer = new QTimer(q);
        moveTimer->setSingleShot(true);
        QObject::connect(moveTimer, SIGNAL(timeout()), q, SLOT(checkBorders()));
    }

    moveTimer->start(triggeredByResize ? 0 : 200);
}

void DialogPrivate::themeChanged()
{
    checkBorders(false);

    const bool translucency = Plasma::Theme::defaultTheme()->windowTranslucencyEnabled();
    // WA_NoSystemBackground is going to fail combined with sliding popups, but is needed
    // when we aren't compositing
    q->setAttribute(Qt::WA_NoSystemBackground, !translucency);
    updateMask();
}

void DialogPrivate::updateMask()
{
    const bool translucency = Plasma::Theme::defaultTheme()->windowTranslucencyEnabled();
    WindowEffects::enableBlurBehind(q->winId(), translucency,
                                    translucency ? background->mask() : QRegion());
    if (translucency) {
        q->clearMask();
    } else {
        q->setMask(background->mask());
    }
}

void DialogPrivate::checkBorders()
{
    checkBorders(true);
}

Plasma::Applet *DialogPrivate::applet()
{
    Extender *extender = qobject_cast<Extender*>(graphicsWidgetPtr.data());
    Plasma::Applet *applet = 0;
    if (extender) {
        if (!extender->d->applet) {
            return 0;
        }
        applet = extender->d->applet.data();
    } else if (graphicsWidgetPtr) {
        QObject *pw = graphicsWidgetPtr.data();

        while ((pw = pw->parent())) {
            applet = dynamic_cast<Plasma::Applet *>(pw);
            if (applet) {
                break;
            }
        }
    }
    return applet;
}

void DialogPrivate::delayedAdjustSize()
{
    q->syncToGraphicsWidget();
}

void DialogPrivate::checkBorders(bool updateMaskIfNeeded)
{
    if (resizeChecksWithBorderCheck) {
        background->resizeFrame(q->size());
    }

    QGraphicsWidget *graphicsWidget = graphicsWidgetPtr.data();
    const FrameSvg::EnabledBorders currentBorders = background->enabledBorders();
    FrameSvg::EnabledBorders borders = FrameSvg::AllBorders;

    Extender *extender = qobject_cast<Extender*>(graphicsWidget);
    Plasma::Applet *applet = this->applet();

    //used to remove borders at the edge of the desktop
    QDesktopWidget *desktop = QApplication::desktop();
    QRect avail = desktop->availableGeometry(desktop->screenNumber(q));
    QRect screenGeom = desktop->screenGeometry(desktop->screenNumber(q));
    QRect dialogGeom = q->geometry();

    qreal topHeight(0);
    qreal leftWidth(0);
    qreal rightWidth(0);
    qreal bottomHeight(0);

    //decide about disabling the border attached to the panel
    if (applet) {
        background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);

        switch (applet->location()) {
        case BottomEdge:
            if (applet->containment() &&
                dialogGeom.bottom() + 2 >= screenGeom.bottom() - applet->containment()->size().height() &&
                dialogGeom.width() <= applet->containment()->size().width()) {
                borders &= ~FrameSvg::BottomBorder;
                leftWidth = 0;
                rightWidth = 0;
                bottomHeight = 0;
            }
        break;

        case TopEdge:
            if (applet->containment() &&
                dialogGeom.top() <= screenGeom.top() + applet->containment()->size().height() &&
                dialogGeom.width() <= applet->containment()->size().width()) {
                borders &= ~FrameSvg::TopBorder;
                topHeight = 0;
                leftWidth = 0;
                rightWidth = 0;
            }
        break;

        case LeftEdge:
            if (applet->containment() &&
                dialogGeom.left() <= screenGeom.left() + applet->containment()->size().width() &&
                dialogGeom.height() <= applet->containment()->size().height()) {
                borders &= ~FrameSvg::LeftBorder;
                leftWidth = 0;
                rightWidth = 0;
            }
        break;

        case RightEdge:
            if (applet->containment() &&
                dialogGeom.right() + 2 >= screenGeom.right() - applet->containment()->size().width() &&
                dialogGeom.height() <= applet->containment()->size().height()) {
                borders &= ~FrameSvg::RightBorder;
                leftWidth = 0;
                rightWidth = 0;
            }
        break;

        default:
        break;
        }
    }

    //decide if to disable the other borders
    if (!extender && q->isVisible()) {
        QRect geom;
        if (applet) {
            geom = screenGeom;
        } else {
            geom = avail;
        }

        if (dialogGeom.left() <= geom.left()) {
            borders &= ~FrameSvg::LeftBorder;
        }
        if (dialogGeom.top() <= geom.top()) {
            borders &= ~FrameSvg::TopBorder;
        }
        //FIXME: that 2 pixels offset has probably something to do with kwin
        if (dialogGeom.right() + 2 > geom.right()) {
            borders &= ~FrameSvg::RightBorder;
        }
        if (dialogGeom.bottom() + 2 > geom.bottom()) {
            borders &= ~FrameSvg::BottomBorder;
        }
    }

    background->setEnabledBorders(borders);

    if (!extender) {
        background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);
    }

    //kDebug() << leftWidth << topHeight << rightWidth << bottomHeight;
    q->setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    if (resizeChecksWithBorderCheck) {
        updateResizeCorners();
        updateMask();
        q->update();
    } else if (currentBorders != borders) {
        if (updateMaskIfNeeded) {
            updateMask();
        }

        q->update();
    } 

    resizeChecksWithBorderCheck = false;
}

void Dialog::syncToGraphicsWidget()
{
    d->adjustViewTimer->stop();
    QGraphicsWidget *graphicsWidget = d->graphicsWidgetPtr.data();
    if (d->view && graphicsWidget && d->resizeStartCorner != -1) {
        const int prevStartCorner = d->resizeStartCorner;
        d->resizeStartCorner = -1;
        QSize prevSize = size();
        /*
        kDebug() << "Widget size:" << graphicsWidget->size()
                 << "| Widget size hint:" << graphicsWidget->effectiveSizeHint(Qt::PreferredSize)
                 << "| Widget minsize hint:" << graphicsWidget->minimumSize()
                 << "| Widget maxsize hint:" << graphicsWidget->maximumSize()
                 << "| Widget bounding rect:" << graphicsWidget->sceneBoundingRect();
        */
        //set the sizehints correctly:
        int left, top, right, bottom;
        getContentsMargins(&left, &top, &right, &bottom);

        QDesktopWidget *desktop = QApplication::desktop();
        QSize maxSize = desktop->availableGeometry(desktop->screenNumber(this)).size();

        graphicsWidget->setMaximumSize(maxSize - QSize(left + right, top + bottom).boundedTo(graphicsWidget->effectiveSizeHint(Qt::MaximumSize).toSize()));

        setMinimumSize(-1, -1);
        setMaximumSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX);

        QSize newSize(qMin(int(graphicsWidget->size().width()) + left + right, maxSize.width()),
               qMin(int(graphicsWidget->size().height()) + top + bottom, maxSize.height()));

        QSize newMinimumSize(qMin(int(graphicsWidget->minimumSize().width()) + left + right, maxSize.width()),
                       qMin(int(graphicsWidget->minimumSize().height()) + top + bottom, maxSize.height()));


        QSize newMaximumSize(qMin(int(graphicsWidget->maximumSize().width()) + left + right, maxSize.width()),
                       qMin(int(graphicsWidget->maximumSize().height()) + top + bottom, maxSize.height()));


        Plasma::Applet *applet = d->applet();
        if (applet) {
            QRect currentGeometry(geometry());
            currentGeometry.setSize(newSize);
            if (applet->location() == Plasma::TopEdge ||
                applet->location() == Plasma::LeftEdge) {
                currentGeometry.setSize(newSize);
            } else if (applet->location() == Plasma::RightEdge) {
                currentGeometry.moveTopRight(geometry().topRight());
            //BottomEdge and floating
            } else {
                currentGeometry.moveBottomLeft(geometry().bottomLeft());
            }
            setGeometry(currentGeometry);
        } else {
            resize(newSize);
        }

        setMinimumSize(newMinimumSize);
        setMaximumSize(newMaximumSize);


        updateGeometry();

        //reposition and resize the view.
        //force a valid rect, otherwise it will take up the whole scene
        QRectF sceneRect(graphicsWidget->sceneBoundingRect());

        sceneRect.setWidth(qMax(qreal(1), sceneRect.width()));
        sceneRect.setHeight(qMax(qreal(1), sceneRect.height()));
        d->view->setSceneRect(sceneRect);

        //d->view->resize(graphicsWidget->size().toSize());
        d->view->centerOn(graphicsWidget);

        //if the view resized and a border is disabled move the dialog to make sure it will still look attached to panel/screen edge
        qreal topHeight;
        qreal leftWidth;
        qreal rightWidth;
        qreal bottomHeight;

        d->background->getMargins(leftWidth, topHeight, rightWidth, bottomHeight);

        if (size() != prevSize) {
            //the size of the dialog has changed, emit the signal:
            emit dialogResized();
        }

        d->resizeStartCorner = prevStartCorner;
    }
}

int DialogPrivate::calculateWidthForHeightAndRatio(int height, qreal ratio)
{
    switch (aspectRatioMode) {
        case KeepAspectRatio:
            return qRound(height * ratio);
            break;
        case Square:
            return height;
            break;
        case ConstrainedSquare:
            return height;
            break;
        default:
            return -1;
    }
}

Dialog::Dialog(QWidget *parent, Qt::WindowFlags f)
    : QWidget(parent, f | Qt::FramelessWindowHint),
      d(new DialogPrivate(this))
{
    setAttribute(Qt::WA_TranslucentBackground);
    d->background = new FrameSvg(this);
    d->background->setImagePath("dialogs/background");
    d->background->setEnabledBorders(FrameSvg::AllBorders);
    d->background->resizeFrame(size());

    QPalette pal = palette();
    pal.setColor(backgroundRole(), Qt::transparent);
    setPalette(pal);
    WindowEffects::overrideShadow(winId(), true);

    d->adjustViewTimer = new QTimer(this);
    d->adjustViewTimer->setSingleShot(true);
    connect(d->adjustViewTimer, SIGNAL(timeout()), this, SLOT(syncToGraphicsWidget()));

    d->adjustSizeTimer = new QTimer(this);
    d->adjustSizeTimer->setSingleShot(true);
    connect(d->adjustSizeTimer, SIGNAL(timeout()), this, SLOT(delayedAdjustSize()));

    connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(update()));

    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeChanged()));
    d->themeChanged();

    setMouseTracking(true);
}

Dialog::~Dialog()
{
    delete d;
}

void Dialog::paintEvent(QPaintEvent *e)
{
    QPainter p(this);
    p.setCompositionMode(QPainter::CompositionMode_Source);
    d->background->paintFrame(&p, e->rect(), e->rect());
}

void Dialog::mouseMoveEvent(QMouseEvent *event)
{
    if (event->modifiers() == Qt::AltModifier) {
        unsetCursor();
    } else if (d->resizeAreas[Dialog::NorthEast].contains(event->pos())) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (d->resizeAreas[Dialog::NorthWest].contains(event->pos())) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (d->resizeAreas[Dialog::SouthEast].contains(event->pos())) {
        setCursor(Qt::SizeFDiagCursor);
    } else if (d->resizeAreas[Dialog::SouthWest].contains(event->pos())) {
        setCursor(Qt::SizeBDiagCursor);
    } else if (!(event->buttons() & Qt::LeftButton)) {
        unsetCursor();
    }

    // here we take care of resize..
    if (d->resizeStartCorner != Dialog::NoCorner) {
        int newWidth;
        int newHeight;
        QPoint position;

        qreal aspectRatio = (qreal)width() / (qreal)height();

        switch(d->resizeStartCorner) {
            case Dialog::NorthEast:
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height() - event->y()));
                newWidth = d->calculateWidthForHeightAndRatio(newHeight, aspectRatio);
                if (newWidth == -1) {
                    newWidth = qMin(maximumWidth(), qMax(minimumWidth(), event->x()));
                }
                position = QPoint(x(), y() + height() - newHeight);
                break;
            case Dialog::NorthWest:
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height() - event->y()));
                newWidth = d->calculateWidthForHeightAndRatio(newHeight, aspectRatio);
                if (newWidth == -1) {
                    newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width() - event->x()));
                }
                position = QPoint(x() + width() - newWidth, y() + height() - newHeight);
                break;
            case Dialog::SouthWest:
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), event->y()));
                newWidth = d->calculateWidthForHeightAndRatio(newHeight, aspectRatio);
                if (newWidth == -1) {
                    newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width() - event->x()));
                }
                position = QPoint(x() + width() - newWidth, y());
                break;
            case Dialog::SouthEast:
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), event->y()));
                newWidth = d->calculateWidthForHeightAndRatio(newHeight, aspectRatio);
                if (newWidth == -1) {
                    newWidth = qMin(maximumWidth(), qMax(minimumWidth(), event->x()));
                }
                position = QPoint(x(), y());
                break;
             default:
                newHeight = qMin(maximumHeight(), qMax(minimumHeight(), height()));
                newWidth = d->calculateWidthForHeightAndRatio(newHeight, aspectRatio);
                if (newWidth == -1) {
                    newWidth = qMin(maximumWidth(), qMax(minimumWidth(), width()));
                }
                position = QPoint(x(), y());
                break;
        }

        if ((newWidth >= minimumSize().width()) && (newHeight >= minimumSize().height())) {
            setGeometry(QRect(position, QSize(newWidth, newHeight)));
        }
    }

    QWidget::mouseMoveEvent(event);
}

void Dialog::mousePressEvent(QMouseEvent *event)
{
    if (d->resizeAreas[Dialog::NorthEast].contains(event->pos())) {
        d->resizeStartCorner = Dialog::NorthEast;
    } else if (d->resizeAreas[Dialog::NorthWest].contains(event->pos())) {
        d->resizeStartCorner = Dialog::NorthWest;
    } else if (d->resizeAreas[Dialog::SouthEast].contains(event->pos())) {
        d->resizeStartCorner = Dialog::SouthEast;
    } else if (d->resizeAreas[Dialog::SouthWest].contains(event->pos())) {
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

void Dialog::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event)
    //kDebug();
    d->scheduleBorderCheck(true);

    if (d->resizeStartCorner != -1 && d->view && d->graphicsWidgetPtr) {
        QGraphicsWidget *graphicsWidget = d->graphicsWidgetPtr.data();
        graphicsWidget->resize(d->view->size());

        QRectF sceneRect(graphicsWidget->sceneBoundingRect());
        sceneRect.setWidth(qMax(qreal(1), sceneRect.width()));
        sceneRect.setHeight(qMax(qreal(1), sceneRect.height()));
        d->view->setSceneRect(sceneRect);
        d->view->centerOn(graphicsWidget);
    }
}

void DialogPrivate::updateResizeCorners()
{
    const int resizeAreaMargin = 20;
    const QRect r = q->rect();
    const FrameSvg::EnabledBorders borders = background->enabledBorders();

    // IMPLEMENTATION NOTE: we set resize corners for the corners set, but also
    // for the complimentary corners if we've cut out an edge of our SVG background
    // which implies we are up against an immovable edge (e.g. a screen edge)

    resizeAreas.clear();
    if (resizeCorners & Dialog::NorthEast ||
        (resizeCorners & Dialog::NorthWest && !(borders & FrameSvg::LeftBorder)) ||
        (resizeCorners & Dialog::SouthEast && !(borders & FrameSvg::BottomBorder))) {
        resizeAreas[Dialog::NorthEast] = QRect(r.right() - resizeAreaMargin, 0,
                                               resizeAreaMargin, resizeAreaMargin);
    }

    if (resizeCorners & Dialog::NorthWest ||
        (resizeCorners & Dialog::NorthEast && !(borders & FrameSvg::RightBorder)) ||
        (resizeCorners & Dialog::SouthWest && !(borders & FrameSvg::BottomBorder))) {
        resizeAreas[Dialog::NorthWest] = QRect(0, 0, resizeAreaMargin, resizeAreaMargin);
    }

    if (resizeCorners & Dialog::SouthEast ||
        (resizeCorners & Dialog::SouthWest && !(borders & FrameSvg::LeftBorder)) ||
        (resizeCorners & Dialog::NorthEast && !(borders & FrameSvg::TopBorder))) {
        resizeAreas[Dialog::SouthEast] = QRect(r.right() - resizeAreaMargin,
                                               r.bottom() - resizeAreaMargin,
                                               resizeAreaMargin, resizeAreaMargin);
    }

    if (resizeCorners & Dialog::SouthWest ||
        (resizeCorners & Dialog::SouthEast && !(borders & FrameSvg::RightBorder)) ||
        (resizeCorners & Dialog::NorthWest && !(borders & FrameSvg::TopBorder))) {
        resizeAreas[Dialog::SouthWest] = QRect(0, r.bottom() - resizeAreaMargin,
                                               resizeAreaMargin, resizeAreaMargin);
    }
}

void Dialog::setGraphicsWidget(QGraphicsWidget *widget)
{
    if (d->graphicsWidgetPtr) {
        d->graphicsWidgetPtr.data()->removeEventFilter(this);
    }

    d->graphicsWidgetPtr = widget;

    if (widget) {
        if (!layout()) {
            QVBoxLayout *lay = new QVBoxLayout(this);
            lay->setMargin(0);
            lay->setSpacing(0);
        }

        d->checkBorders();

        if (!d->view) {
            d->view = new QGraphicsView(this);
            d->view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            d->view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
            d->view->setFrameShape(QFrame::NoFrame);
            d->view->viewport()->setAutoFillBackground(false);
            layout()->addWidget(d->view);
        }

        d->view->setScene(widget->scene());

        //try to have the proper size -before- showing the dialog
        d->view->centerOn(widget);
        if (widget->layout()) {
            widget->layout()->activate();
        }
        static_cast<QGraphicsLayoutItem *>(widget)->updateGeometry();
        widget->resize(widget->size().expandedTo(widget->effectiveSizeHint(Qt::MinimumSize)));

        syncToGraphicsWidget();

        //d->adjustSizeTimer->start(150);

        widget->installEventFilter(this);
        d->view->installEventFilter(this);
    } else {
        delete d->view;
        d->view = 0;
    }
}

//KDE5 FIXME: should be const
QGraphicsWidget *Dialog::graphicsWidget()
{
    return d->graphicsWidgetPtr.data();
}

bool Dialog::eventFilter(QObject *watched, QEvent *event)
{
    if (d->resizeStartCorner == Dialog::NoCorner && watched == d->graphicsWidgetPtr.data() &&
        (event->type() == QEvent::GraphicsSceneResize || event->type() == QEvent::GraphicsSceneMove)) {
        d->adjustViewTimer->start(150);
    }

    // when moving the cursor with a 45° angle from the outside
    // to the inside passing over a resize angle the cursor changes its
    // shape to a resize cursor. As a side effect this is the only case
    // when the cursor immediately enters the view without giving
    // the dialog the chance to restore the original cursor shape.
    if (event->type() == QEvent::Enter && watched == d->view) {
        unsetCursor();
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
    d->checkBorders();
    d->updateResizeCorners();

    QGraphicsWidget *graphicsWidget = d->graphicsWidgetPtr.data();
    if (graphicsWidget &&
        ((d->view && graphicsWidget->size().toSize() != d->view->size()) ||
         d->oldGraphicsWidgetMinimumSize != graphicsWidget->minimumSize() ||
         d->oldGraphicsWidgetMaximumSize != graphicsWidget->maximumSize())) {
        //here have to be done immediately, ideally should have to be done -before- shwing, but is not possible to catch show() so early
        syncToGraphicsWidget();
        d->oldGraphicsWidgetMinimumSize = graphicsWidget->minimumSize().toSize();
        d->oldGraphicsWidgetMaximumSize = graphicsWidget->maximumSize().toSize();
    }

    if (d->view) {
        d->view->setFocus();
    }

    if (graphicsWidget) {
        graphicsWidget->setFocus();
    }

    emit dialogVisible(true);
    WindowEffects::overrideShadow(winId(), true);
}

void Dialog::focusInEvent(QFocusEvent *event)
{
    Q_UNUSED(event)

    if (d->view) {
        d->view->setFocus();
    }

    QGraphicsWidget *graphicsWidget = d->graphicsWidgetPtr.data();
    if (graphicsWidget) {
        graphicsWidget->setFocus();
    }
}

void Dialog::moveEvent(QMoveEvent *event)
{
    Q_UNUSED(event)
    //kDebug();
    d->scheduleBorderCheck();
}

void Dialog::setResizeHandleCorners(ResizeCorners corners)
{
    if ((d->resizeCorners != corners) && (aspectRatioMode() != FixedSize)) {
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
    if (!KWindowSystem::compositingActive()) {
        hide();
        return;
    }

    Location location = Desktop;
    switch (direction) {
    case Down:
        location = BottomEdge;
        break;
    case Right:
        location = RightEdge;
        break;
    case Left:
        location = LeftEdge;
        break;
    case Up:
        location = TopEdge;
        break;
    default:
        break;
    }

    Plasma::WindowEffects::slideWindow(this, location);
    hide();
}

void Dialog::animatedShow(Plasma::Direction direction)
{
    if (!KWindowSystem::compositingActive()) {
        show();
        return;
    }

    //copied to not add new api
    Location location = Desktop;
    switch (direction) {
    case Up:
        location = BottomEdge;
        break;
    case Left:
        location = RightEdge;
        break;
    case Right:
        location = LeftEdge;
        break;
    case Down:
        location = TopEdge;
        break;
    default:
        break;
    }

    if (KWindowSystem::compositingActive()) {
        Plasma::WindowEffects::slideWindow(this, location);
    }

    show();
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

Plasma::AspectRatioMode Dialog::aspectRatioMode() const
{
    return d->aspectRatioMode;
}

void Dialog::setAspectRatioMode(Plasma::AspectRatioMode mode)
{
    if (mode == FixedSize) {
        setResizeHandleCorners(NoCorner);
    }

    d->aspectRatioMode = mode;
}

}
#include "dialog.moc"
