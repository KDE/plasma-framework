/*
*   Copyright 2011 by Aaron Seigo <aseigo@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2, 
*   or (at your option) any later version.
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

#include "dialogshadows_p.h"

#include <QWidget>
#include <QPainter>

#ifdef Q_WS_X11
#include <QX11Info>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#include <kdebug.h>
#include <kglobal.h>

class DialogShadows::Private
{
public:
    Private(DialogShadows *shadows)
        : q(shadows),
          m_managePixmaps(false)
    {
    }

    ~Private()
    {
        clearPixmaps();
    }

    void clearPixmaps();
    void setupPixmaps();
    void initPixmap(const QString &element);
    void updateShadow(const QWidget *window, Plasma::FrameSvg::EnabledBorders);
    void clearShadow(const QWidget *window);
    void updateShadows();
    void windowDestroyed(QObject *deletedObject);
    void setupData(Plasma::FrameSvg::EnabledBorders enabledBorders);

    DialogShadows *q;
    QList<QPixmap> m_shadowPixmaps;
    QList<QPixmap> m_emptyShadowPixmaps;
    QHash<Plasma::FrameSvg::EnabledBorders, QVector<unsigned long> > data;
    QHash<const QWidget *, Plasma::FrameSvg::EnabledBorders> m_windows;
    bool m_managePixmaps;
};

class DialogShadowsSingleton
{
public:
    DialogShadowsSingleton()
    {
    }

   DialogShadows self;
};

K_GLOBAL_STATIC(DialogShadowsSingleton, privateDialogShadowsSelf)

DialogShadows::DialogShadows(QObject *parent)
    : Plasma::Svg(parent),
      d(new Private(this))
{
    setImagePath("dialogs/background");
    connect(this, SIGNAL(repaintNeeded()), this, SLOT(updateShadows()));
}

DialogShadows *DialogShadows::self()
{
    return &privateDialogShadowsSelf->self;
}

void DialogShadows::addWindow(const QWidget *window, Plasma::FrameSvg::EnabledBorders enabledBorders)
{
    if (!window || !window->isWindow()) {
        return;
    }

    d->m_windows[window] = enabledBorders;
    d->updateShadow(window, enabledBorders);
    connect(window, SIGNAL(destroyed(QObject*)),
            this, SLOT(windowDestroyed(QObject*)), Qt::UniqueConnection);
}

void DialogShadows::removeWindow(const QWidget *window)
{
    if (!d->m_windows.contains(window)) {
        return;
    }

    d->m_windows.remove(window);
    disconnect(window, 0, this, 0);
    d->clearShadow(window);

    if (d->m_windows.isEmpty()) {
        d->clearPixmaps();
    }
}

void DialogShadows::Private::windowDestroyed(QObject *deletedObject)
{
    m_windows.remove(static_cast<QWidget *>(deletedObject));

    if (m_windows.isEmpty()) {
        clearPixmaps();
    }
}

void DialogShadows::Private::updateShadows()
{
    setupPixmaps();
    QHash<const QWidget *, Plasma::FrameSvg::EnabledBorders>::const_iterator i;
    for (i = m_windows.constBegin(); i != m_windows.constEnd(); ++i) {
        updateShadow(i.key(), i.value());
    }
}

void DialogShadows::Private::initPixmap(const QString &element)
{
#ifdef Q_WS_X11
    QPixmap pix = q->pixmap(element);
    if (!pix.isNull() && pix.handle() == 0) {
        Pixmap xPix = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), pix.width(), pix.height(), 32);
        QPixmap tempPix = QPixmap::fromX11Pixmap(xPix, QPixmap::ExplicitlyShared);
        tempPix.fill(Qt::transparent);
        QPainter p(&tempPix);
        p.drawPixmap(QPoint(0, 0), pix);
        m_shadowPixmaps << tempPix;
        m_managePixmaps = true;

        //make an empty pixmap for when the border is sisabled
        QSize size = q->elementSize(element);
        xPix = XCreatePixmap(QX11Info::display(), QX11Info::appRootWindow(), size.width(), size.height(), 32);
        tempPix = QPixmap::fromX11Pixmap(xPix, QPixmap::ExplicitlyShared);
        tempPix.fill(Qt::transparent);
        m_emptyShadowPixmaps << tempPix;
    } else {
        m_shadowPixmaps << pix;
        m_emptyShadowPixmaps << pix;
    }
#endif
}

void DialogShadows::Private::setupPixmaps()
{
    clearPixmaps();
    initPixmap("shadow-top");
    initPixmap("shadow-topright");
    initPixmap("shadow-right");
    initPixmap("shadow-bottomright");
    initPixmap("shadow-bottom");
    initPixmap("shadow-bottomleft");
    initPixmap("shadow-left");
    initPixmap("shadow-topleft");
}


void DialogShadows::Private::setupData(Plasma::FrameSvg::EnabledBorders enabledBorders)
{
#ifdef Q_WS_X11
    /*foreach (const QPixmap &pixmap, m_shadowPixmaps) {
        data[enabledBorders] << pixmap.handle();
    }*/

    //shadow-top
    if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        data[enabledBorders] << m_shadowPixmaps[0].handle();
    } else {
        data[enabledBorders] << m_emptyShadowPixmaps[0].handle();
    }

    //shadow-topright
    if (enabledBorders & Plasma::FrameSvg::TopBorder &&
        enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << m_shadowPixmaps[1].handle();
    } else {
        data[enabledBorders] << m_emptyShadowPixmaps[1].handle();
    }

    //shadow-right
    if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << m_shadowPixmaps[2].handle();
    } else {
        data[enabledBorders] << m_emptyShadowPixmaps[2].handle();
    }

    //shadow-bottomright
    if (enabledBorders & Plasma::FrameSvg::BottomBorder &&
        enabledBorders & Plasma::FrameSvg::RightBorder) {
        data[enabledBorders] << m_shadowPixmaps[3].handle();
    } else {
        data[enabledBorders] << m_emptyShadowPixmaps[3].handle();
    }

    //shadow-bottom
    if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        data[enabledBorders] << m_shadowPixmaps[4].handle();
    } else {
        data[enabledBorders] << m_emptyShadowPixmaps[4].handle();
    }

    //shadow-bottomleft
    if (enabledBorders & Plasma::FrameSvg::BottomBorder &&
        enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << m_shadowPixmaps[5].handle();
    } else {
        data[enabledBorders] << m_emptyShadowPixmaps[5].handle();
    }

    //shadow-left
    if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << m_shadowPixmaps[6].handle();
    } else {
        data[enabledBorders] << m_emptyShadowPixmaps[6].handle();
    }

    //shadow-topleft
    if (enabledBorders & Plasma::FrameSvg::TopBorder &&
        enabledBorders & Plasma::FrameSvg::LeftBorder) {
        data[enabledBorders] << m_shadowPixmaps[7].handle();
    } else {
        data[enabledBorders] << m_emptyShadowPixmaps[7].handle();
    }
#endif

    int left, top, right, bottom = 0;

    QSize marginHint;
    if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        marginHint = q->elementSize("shadow-hint-top-margin");
        kDebug() << "top margin hint is:" << marginHint;
        if (marginHint.isValid()) {
            top = marginHint.height();
        } else {
            top = m_shadowPixmaps[0].height(); // top
        }
    } else {
        top = m_shadowPixmaps[7].height(); // topleft
    }

    if (enabledBorders & Plasma::FrameSvg::RightBorder) {
        marginHint = q->elementSize("shadow-hint-right-margin");
        kDebug() << "right margin hint is:" << marginHint;
        if (marginHint.isValid()) {
            right = marginHint.width();
        } else {
            right = m_shadowPixmaps[2].width(); // right
        }
    } else {
        right = m_shadowPixmaps[1].width(); // topright
    }

    if (enabledBorders & Plasma::FrameSvg::BottomBorder) {
        marginHint = q->elementSize("shadow-hint-bottom-margin");
        if (marginHint.isValid()) {
            bottom = marginHint.height();
        } else {
            bottom = m_shadowPixmaps[4].height(); // bottom
        }
    } else {
        bottom = m_shadowPixmaps[5].height(); // bottomleft
    }

    if (enabledBorders & Plasma::FrameSvg::LeftBorder) {
        marginHint = q->elementSize("shadow-hint-left-margin");
        if (marginHint.isValid()) {
            left = marginHint.width();
        } else {
            left = m_shadowPixmaps[6].width(); // left
        }
    } else {
        left = m_shadowPixmaps[7].width(); // topleft
    }

    data[enabledBorders] << top << right << bottom << left;
}

void DialogShadows::Private::clearPixmaps()
{
#ifdef Q_WS_X11
    if (m_managePixmaps) {
        foreach (const QPixmap &pixmap, m_shadowPixmaps) {
            XFreePixmap(QX11Info::display(), pixmap.handle());
        }
        foreach (const QPixmap &pixmap, m_emptyShadowPixmaps) {
            XFreePixmap(QX11Info::display(), pixmap.handle());
        }
        m_managePixmaps = false;
    }
#endif
    m_shadowPixmaps.clear();
    m_emptyShadowPixmaps.clear();
    data.clear();
}

void DialogShadows::Private::updateShadow(const QWidget *window, Plasma::FrameSvg::EnabledBorders enabledBorders)
{
#ifdef Q_WS_X11
    if (m_shadowPixmaps.size() == 0) {
        setupPixmaps();
    }

    if (!data.contains(enabledBorders)) {
        setupData(enabledBorders);
    }

    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_NET_WM_SHADOW", False);

    //kDebug() << "going to set the shadow of" << winId() << "to" << data;
    XChangeProperty(dpy, window->winId(), atom, XA_CARDINAL, 32, PropModeReplace,
                    reinterpret_cast<const unsigned char *>(data[enabledBorders].constData()), data[enabledBorders].size());
#endif
}

void DialogShadows::Private::clearShadow(const QWidget *window)
{
#ifdef Q_WS_X11
    Display *dpy = QX11Info::display();
    Atom atom = XInternAtom(dpy, "_KDE_NET_WM_SHADOW", False);
    XDeleteProperty(dpy, window->winId(), atom);
#endif
}

#include "dialogshadows_p.moc"

