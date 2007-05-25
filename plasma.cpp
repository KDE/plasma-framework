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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <plasma.h>

namespace Plasma
{

int plasmaIds = 0;

void setMinId(int minId)
{
    plasmaIds = minId + 1;
}

int uniqueId()
{
    return ++plasmaIds;
}

Direction locationToDirection(Location location)
{
    switch (location)
    {
        case Floating:
        case Desktop:
        case TopEdge:
            //TODO: should we be smarter for floating and planer?
            //      perhaps we should take a QRect and/or QPos as well?
            return Down;
        case BottomEdge:
            return Up;
        case LeftEdge:
            return Right;
        case RightEdge:
            return Left;
    }

    return Down;
}

} // Plasma namespace
