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

#ifndef PLASMA_WINDOWEFFECTS_H
#define PLASMA_WINDOWEFFECTS_H

#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

#include <KDebug>

#include <plasma/plasma_export.h>
#include <plasma/plasma.h>

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <QX11Info>
#endif

/** @headerfile plasma/windoweffect.h <Plasma/PaintUtils> */

namespace Plasma
{

/**
 * Namespace for all window effects for Plasma/KWin interaction
 * @since 4.4
 */
namespace WindowEffects
{
    /**
     * Mark a window as sliding from screen edge
     *
     * @arg id of the window on which we want to apply the effect
     * @arg location edge of the screen from which we want the slifing effect.
     *               Desktop and FLoating won't have effect.
     * @arg offset distance in pixels from the screen edge defined by location
     * @since 4.4
     */
    PLASMA_EXPORT void slideWindow(WId id, Plasma::Location location, int offset);

    /**
     * Mark a window as sliding from screen edge
     * This is an overloaded member function provided for convenience
     *
     * @arg widget QWidget corresponding to the top level window we want to animate
     * @arg location edge of the screen from which we want the slifing effect.
     *               Desktop and FLoating won't have effect.
     * @since 4.4
     */
    PLASMA_EXPORT void slideWindow(QWidget *widget, Plasma::Location location);
}

} // namespace Plasma

#endif

