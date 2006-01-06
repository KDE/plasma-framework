/*
 *   Copyright (C) 2005 by Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef PLASMA_DEFS_H
#define PLASMA_DEFS_H

#include <kdemacros.h>

namespace Plasma
{

enum AppletConstraint { NoConstraint = 0,
                        Width = 1,
                        Height = 2,
                        MaxAppletConstraint = Height };

enum Direction { Down = 0,
                 Up,
                 Left,
                 Right };

enum ScreenEdge { Floating = 0, Desktop,
                  TopEdge, BottomEdge, LeftEdge, RightEdge };

KDE_EXPORT void setMinId(int minId);
KDE_EXPORT int uniqueId();
KDE_EXPORT Direction edgeToPopupDirection(ScreenEdge edge);

} // Plasma namespace

#endif // multiple inclusion guard
