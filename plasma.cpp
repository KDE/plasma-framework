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

#include <plasma.h>

namespace Plasma
{

int plasmaIDs = 0;

void setMinID(int minID)
{
    plasmaIDs = minID + 1;
}

int uniqueID()
{
    return ++plasmaIDs;
}

Direction edgeToPopupDirection(ScreenEdge edge)
{
    switch (edge)
    {
        case Floating:
        case Desktop:
        case TopEdge:
            return Down;
        case BottomEdge:
            return Up;
        case LeftEdge:
            return Right;
        case RightEdge:
            return Left;
    }
}

} // Plasma namespace
