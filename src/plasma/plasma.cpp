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

#include "moc_plasma.cpp"
