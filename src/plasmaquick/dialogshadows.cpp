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

#include <QWindow>
#include <QPainter>
#include <config-plasma.h>

#include <KWindowSystem>
#include <KWindowEffects>
#if HAVE_X11
#include <QX11Info>
#include <X11/Xatom.h>
#include <X11/Xlib.h>
#include <X11/Xlib-xcb.h>
#include <fixx11h.h>
#endif


class DialogShadowsSingleton
{
public:
    DialogShadowsSingleton()
    {
    }

    DialogShadows self;
};

Q_GLOBAL_STATIC(DialogShadowsSingleton, privateDialogShadowsSelf)

DialogShadows::DialogShadows(QObject *parent, const QString &prefix)
    : Plasma::Svg(parent)
{
    setImagePath(prefix);
    connect(this, &Plasma::Svg::repaintNeeded, this, &DialogShadows::updateShadows);
    updateShadows();
}

DialogShadows::~DialogShadows()
{
}

DialogShadows *DialogShadows::self()
{
    return &privateDialogShadowsSelf->self;
}

void DialogShadows::addWindow(QWindow *window, Plasma::FrameSvg::EnabledBorders enabledBorders)
{
    KWindowEffects::ShadowData::EnabledBorders borders;
    //FIXME
    if (enabledBorders & Plasma::FrameSvg::TopBorder) {
        borders |= KWindowEffects::ShadowData::AllBorders;
    }

    m_shadow.decorateWindow(window);
}

void DialogShadows::removeWindow(QWindow *window)
{
    m_shadow.clearWindow(window);
}

void DialogShadows::updateShadows()
{
    const QImage top = pixmap(QStringLiteral("shadow-top")).toImage();
    const QImage left = pixmap(QStringLiteral("shadow-left")).toImage();
    const QImage right = pixmap(QStringLiteral("shadow-right")).toImage();
    const QImage bottom = pixmap(QStringLiteral("shadow-bottom")).toImage();
    const QImage topLeft = pixmap(QStringLiteral("shadow-topleft")).toImage();
    const QImage topRight = pixmap(QStringLiteral("shadow-topright")).toImage();
    const QImage bottomLeft = pixmap(QStringLiteral("shadow-bottomleft")).toImage();
    const QImage bottomRight = pixmap(QStringLiteral("shadow-bottomright")).toImage();

    int leftMargin, topMargin, rightMargin, bottomMargin = 0;

    QSize marginHint;
    marginHint = elementSize(QStringLiteral("shadow-hint-top-margin"));
    if (marginHint.isValid()) {
        topMargin = marginHint.height();
    } else {
        topMargin = top.height();
    }

    marginHint = elementSize(QStringLiteral("shadow-hint-left-margin"));
    if (marginHint.isValid()) {
        leftMargin = marginHint.width();
    } else {
        leftMargin = left.width();
    }

    marginHint = elementSize(QStringLiteral("shadow-hint-right-margin"));
    if (marginHint.isValid()) {
        rightMargin = marginHint.width();
    } else {
        rightMargin = right.width();
    }

    marginHint = elementSize(QStringLiteral("shadow-hint-bottom-margin"));
    if (marginHint.isValid()) {
        bottomMargin = marginHint.height();
    } else {
        bottomMargin = bottom.height();
    }

    const QMargins margins(leftMargin, topMargin, rightMargin, bottomMargin);

    m_shadow.updateShadow(top, left, right, bottom, topLeft, topRight, bottomLeft, bottomRight, margins);
}
