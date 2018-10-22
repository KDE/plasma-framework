/*
 *   Copyright 2005 by Aaron Seigo <aseigo@kde.org>
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

#include <plasma/plasma.h>

#include <QAction>
#include <QMenu>

#include "containment.h"

namespace Plasma
{

Types::Types(QObject *parent)
    : QObject(parent)
{
}

Types::~Types()
{
}

Types::Direction locationToDirection(Types::Location location)
{
    switch (location) {
    case Types::Floating:
    case Types::Desktop:
    case Types::TopEdge:
    case Types::FullScreen:
        //TODO: should we be smarter for floating and planer?
        //      perhaps we should take a QRect and/or QPos as well?
        return Types::Down;
    case Types::BottomEdge:
        return Types::Up;
    case Types::LeftEdge:
        return Types::Right;
    case Types::RightEdge:
        return Types::Left;
    }

    return Types::Down;
}

Types::Direction locationToInverseDirection(Types::Location location)
{
    switch (location) {
    case Types::Floating:
    case Types::Desktop:
    case Types::TopEdge:
    case Types::FullScreen:
        //TODO: should we be smarter for floating and planer?
        //      perhaps we should take a QRect and/or QPos as well?
        return Types::Up;
    case Types::BottomEdge:
        return Types::Down;
    case Types::LeftEdge:
        return Types::Left;
    case Types::RightEdge:
        return Types::Right;
    }

    return Types::Up;
}

} // Plasma namespace
