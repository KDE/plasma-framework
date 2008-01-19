/*
 *   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>
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
#include "tooltip_p.h"

#include <QBitmap>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPixmap>
#include <QTimer>
#include <QGraphicsView>
#ifdef Q_WS_X11
#include <QX11Info>
#endif

#include <kglobal.h>
#include <kwindowsystem.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#include "plasma/plasma.h"

namespace Plasma {

class ToolTip::Private
{
    public:
        Private()
        : label(0)
        , imageLabel(0)
        , preview(0)
        , windowToPreview(0)
        , currentWidget(0)
        , isShown(false)
        , showTimer(0)
        , hideTimer(0)
    { }

    QLabel *label;
    QLabel *imageLabel;
    WindowPreview *preview;
    WId windowToPreview;
    Plasma::Widget *currentWidget;
    bool isShown;
    QTimer *showTimer;
    QTimer *hideTimer;
};

class ToolTipSingleton
{
    public:
        ToolTip self;
};
K_GLOBAL_STATIC( ToolTipSingleton, privateInstance )

ToolTip *ToolTip::self()
{
    return &privateInstance->self;
}

void ToolTip::show(const QPoint &location, Plasma::Widget *widget)
{
    d->currentWidget = widget;
    setData(widget->toolTip());
    move(location.x(), location.y() - sizeHint().height());
    if (d->isShown) {
        // Don't delay if the tooltip is already shown(i.e. moving from one task to another)
        // Qt doesn't seem to like visible tooltips moving though, so hide it and then
        // immediately show it again
        setVisible(false);

        // small delay to prevent unecessary showing when the mouse is moving quickly across items
        // which can be too much for less powerful CPUs to keep up with
        d->showTimer->start(150);
    } else {
        d->showTimer->start(500);
    }
}

void ToolTip::hide()
{
    d->currentWidget = 0;
    d->showTimer->stop();  //Mouse out, stop the timer to show the tooltip
    setVisible(false);
    d->hideTimer->start(250);  //500 ms delay before we are officially "gone" to allow for the time to move between widgets
}

Plasma::Widget *ToolTip::currentWidget() const
{
    return d->currentWidget;
}

//PRIVATE FUNCTIONS
void ToolTip::slotShowToolTip()
{
    QGraphicsView *v = d->currentWidget->view();
    if (v && v->mouseGrabber()) {
        return;
    }

    if( d->windowToPreview != 0 ) {
        // show/hide the preview area
        d->preview->show();
    } else {
        d->preview->hide();
    }
    layout()->activate();

    d->isShown = true;  //ToolTip is visible
    setVisible(true);
}

void ToolTip::slotResetTimer()
{
    if (!isVisible()) { //One might have moused out and back in again
        d->isShown = false;
    }
}

void ToolTip::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    d->preview->setInfo();
}

void ToolTip::mouseReleaseEvent(QMouseEvent* event)
{
    if (rect().contains(event->pos())) {
        hide();
    }
}

ToolTip::ToolTip()
    : QWidget(0)
    , d( new Private )
{
    setWindowFlags(Qt::ToolTip);
    QGridLayout *l = new QGridLayout;
    d->preview = new WindowPreview;
    d->label = new QLabel;
    d->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    d->label->setWordWrap(true);
    d->imageLabel = new QLabel;
    d->imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    l->addWidget(d->preview, 0, 0, 1, 2);
    l->addWidget(d->imageLabel, 1, 0);
    l->addWidget(d->label, 1, 1);
    setLayout(l);

    d->showTimer = new QTimer(this);
    d->showTimer->setSingleShot(true);
    d->hideTimer = new QTimer(this);
    d->hideTimer->setSingleShot(true);

    connect(d->showTimer, SIGNAL(timeout()), SLOT(slotShowToolTip()));
    connect(d->hideTimer, SIGNAL(timeout()), SLOT(slotResetTimer()));
}

void ToolTip::setData(const Plasma::ToolTipData &data)
{
    d->label->setText("<qt><b>" + data.mainText + "</b><br>" +
                        data.subText + "</qt>");
    d->imageLabel->setPixmap(data.image);
    d->windowToPreview = data.windowToPreview;
    d->preview->setWindowId( d->windowToPreview );
}

ToolTip::~ToolTip()
{
    delete d;
}


// A widget which reserves area for window preview and sets hints on the toplevel
// tooltip widget that tells KWin to render the preview in this area. This depends
// on KWin's TaskbarThumbnail compositing effect (which is here detected).

void WindowPreview::setWindowId(WId w)
{
    if (!previewsAvailable()) {
        id = 0;
        return;
    }
    id = w;
    windowSize = QSize();
}

bool WindowPreview::previewsAvailable() const
{
    if (!KWindowSystem::compositingActive()) {
        return false;
    }
#ifdef Q_WS_X11
    // hackish way to find out if KWin has the effect enabled,
    // TODO provide proper support
    Display* dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_WINDOW_PREVIEW", False);
    int cnt;
    Atom* list = XListProperties(dpy, DefaultRootWindow( dpy ), &cnt);
    if (list != NULL) {
        bool ret = ( qFind(list, list + cnt, atom) != list + cnt );
        XFree(list);
        return ret;
    }
#endif
    return false;
}

QSize WindowPreview::sizeHint() const
{
    if (id == 0) {
        return QSize();
    }
    if (!windowSize.isValid()) {
        readWindowSize();
    }
    QSize s = windowSize;
    s.scale(200, 150, Qt::KeepAspectRatio);
    return s;
}

void WindowPreview::readWindowSize() const
{
#ifdef Q_WS_X11
    Window r;
    int x, y;
    unsigned int w, h, b, d;
    if (XGetGeometry(QX11Info::display(), id, &r, &x, &y, &w, &h, &b, &d)) {
        windowSize = QSize( w, h );
    } else {
        windowSize = QSize();
    }
#else
    windowSize = QSize();
#endif
}

void WindowPreview::setInfo()
{
#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_WINDOW_PREVIEW", False);
    if (id == 0) {
        XDeleteProperty(dpy, winId(), atom);
        return;
    }
    if (!windowSize.isValid()) {
        readWindowSize();
    }
    if (!windowSize.isValid()) {
        XDeleteProperty(dpy, winId(), atom);
        return;
    }
    Q_ASSERT( parentWidget()->isWindow()); // parent must be toplevel
    long data[] = { 1, 5, id, x(), y(), width(), height() };
    XChangeProperty(dpy, parentWidget()->winId(), atom, atom, 32, PropModeReplace,
        reinterpret_cast< unsigned char* >( data ), sizeof( data ) / sizeof( data[ 0 ] ));
#endif
}

void ToolTip::resizeEvent(QResizeEvent *)
{
    QBitmap mask(width(), height());
    QPainter painter(&mask);

    mask.fill(Qt::white);

    painter.setBrush(Qt::black);
    painter.setPen(Qt::black);

    painter.drawPath(roundedRectangle(mask.rect(), 10));
    setMask(mask);
}

void ToolTip::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    //Stroke border
    painter.setPen(palette().dark().color());
    painter.drawPath(roundedRectangle(rect(), 10));
}

}
#include "tooltip_p.moc"
