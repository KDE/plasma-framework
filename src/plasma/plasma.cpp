/*
    SPDX-FileCopyrightText: 2005 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include <plasma/plasma.h>

#include <QAction>
#include <QApplication>
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
        // TODO: should we be smarter for floating and planer?
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
        // TODO: should we be smarter for floating and planer?
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

Types::PopupPlacement visualPopupPlacement(Types::PopupPlacement placement, Qt::LayoutDirection layoutDirection)
{
    const bool mirrored = (layoutDirection == Qt::LayoutDirectionAuto) ? qApp->isRightToLeft() : (layoutDirection == Qt::RightToLeft);

    if (!mirrored) {
        return placement;
    }

    switch (placement) {
    case Types::TopPosedLeftAlignedPopup:
        return Types::TopPosedRightAlignedPopup;
    case Types::TopPosedRightAlignedPopup:
        return Types::TopPosedLeftAlignedPopup;
    case Types::LeftPosedTopAlignedPopup:
        return Types::RightPosedTopAlignedPopup;
    case Types::LeftPosedBottomAlignedPopup:
        return Types::RightPosedBottomAlignedPopup;
    case Types::BottomPosedLeftAlignedPopup:
        return Types::BottomPosedRightAlignedPopup;
    case Types::BottomPosedRightAlignedPopup:
        return Types::BottomPosedLeftAlignedPopup;
    case Types::RightPosedTopAlignedPopup:
        return Types::LeftPosedTopAlignedPopup;
    case Types::RightPosedBottomAlignedPopup:
        return Types::LeftPosedBottomAlignedPopup;
    case Types::FloatingPopup:
    default:
        return placement;
    }
}

} // Plasma namespace
