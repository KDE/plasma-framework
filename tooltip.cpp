/*
 *   Copyright 2007 by Dan Meltzer <hydrogen@notyetimplemented.com>
 *   Copyright (C) 2008 by Alexis Ménard <darktears31@gmail.com>
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

#include <KDebug>
#include <KGlobal>
#include <KWindowSystem>
#include <plasma/theme.h>
#include <plasma/panelsvg.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#include "plasma/plasma.h"

namespace Plasma {

class ToolTipPrivate
{
    public:
        ToolTipPrivate(QObject *s)
        : label(0)
        , imageLabel(0)
        , preview(0)
        , windowToPreview(0)
        , source(s)
    { }

    QLabel *label;
    QLabel *imageLabel;
    WindowPreview *preview;
    WId windowToPreview;
    PanelSvg *background;
    QObject *source;
};

void ToolTip::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);
    d->preview->setInfo();
}

void ToolTip::hideEvent(QHideEvent *e)
{
    QWidget::hideEvent(e);
    if (d->source) {
        QMetaObject::invokeMethod(d->source, SLOT(toolTipHidden()));
    }
}

void ToolTip::mouseReleaseEvent(QMouseEvent* event)
{
    if (rect().contains(event->pos())) {
        hide();
    }
}

ToolTip::ToolTip(QObject *source)
    : QWidget(0)
    , d(new ToolTipPrivate(source))
{
    if (source) {
        connect(source, SIGNAL(destroyed(QObject*)), this, SLOT(sourceDestroyed()));
    }

    setWindowFlags(Qt::ToolTip);
    QGridLayout *l = new QGridLayout;
    d->preview = new WindowPreview;
    d->label = new QLabel;
    d->label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    d->label->setWordWrap(true);
    d->imageLabel = new QLabel;
    d->imageLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    d->background = new PanelSvg(this);
    connect(d->background, SIGNAL(repaintNeeded()), this, SLOT(update()));

    l->addWidget(d->preview, 0, 0, 1, 2);
    l->addWidget(d->imageLabel, 1, 0);
    l->addWidget(d->label, 1, 1);
    setLayout(l);
}

ToolTip::~ToolTip()
{
    delete d;
}

void ToolTip::setContent(const ToolTipManager::ToolTipContent &data)
{
    //reset our size
    d->label->setText("<qt><b>" + data.mainText + "</b><br>" + data.subText + "</qt>");
    d->imageLabel->setPixmap(data.image);
    d->windowToPreview = data.windowToPreview;
    d->preview->setWindowId( d->windowToPreview );

    if (isVisible()) {
        resize(sizeHint());
    }
}

void ToolTip::prepareShowing()
{
    if (d->source) {
        QMetaObject::invokeMethod(d->source, SLOT(toolTipAboutToShow()));
    }

    if (d->windowToPreview != 0) {
        // show/hide the preview area
        d->preview->show();
    } else {
        d->preview->hide();
    }

    layout()->activate();
    resize(sizeHint());

    if (isVisible()) {
        d->preview->setInfo();
    } else {
        show();
    }
}

void ToolTip::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent(e);
    d->background->resizePanel(size());

    setMask(d->background->mask());
}

void ToolTip::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setClipRect(e->rect());
    painter.setCompositionMode(QPainter::CompositionMode_Source );
    painter.fillRect(rect(), Qt::transparent);

    d->background->paintPanel(&painter, rect());
}

void ToolTip::sourceDestroyed()
{
    d->source = 0;
}

void ToolTip::updateTheme()
{
    d->background->setImagePath("widgets/tooltip");
    d->background->setEnabledBorders(PanelSvg::AllBorders);

    const int topHeight = d->background->marginSize(Plasma::TopMargin);
    const int leftWidth = d->background->marginSize(Plasma::LeftMargin);
    const int rightWidth = d->background->marginSize(Plasma::RightMargin);
    const int bottomHeight = d->background->marginSize(Plasma::BottomMargin);
    setContentsMargins(leftWidth, topHeight, rightWidth, bottomHeight);

    // Make the tooltip use Plasma's colorscheme
    QPalette plasmaPalette = QPalette();
    plasmaPalette.setColor(QPalette::Window, Plasma::Theme::defaultTheme()->color(Plasma::Theme::BackgroundColor));
    plasmaPalette.setColor(QPalette::WindowText, Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    setAutoFillBackground(true);
    setPalette(plasmaPalette);
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
    readWindowSize();
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
        XDeleteProperty(dpy, parentWidget()->winId(), atom);
        return;
    }
    if (!windowSize.isValid()) {
        readWindowSize();
    }
    if (!windowSize.isValid()) {
        XDeleteProperty(dpy, parentWidget()->winId(), atom);
        return;
    }
    Q_ASSERT( parentWidget()->isWindow()); // parent must be toplevel
    long data[] = { 1, 5, id, x(), y(), width(), height() };
    XChangeProperty(dpy, parentWidget()->winId(), atom, atom, 32, PropModeReplace,
        reinterpret_cast< unsigned char* >( data ), sizeof( data ) / sizeof( data[ 0 ] ));
#endif
}

}
#include "tooltip_p.moc"
