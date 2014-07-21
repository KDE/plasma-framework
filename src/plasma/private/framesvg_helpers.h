/*
 *   Copyright 2014 by Aleix Pol Gonzalez <aleixpol@blue-systems.com>
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

#ifndef FRAMESVG_HELPERS_H
#define FRAMESVG_HELPERS_H

#include "framesvg.h"

namespace Plasma
{

namespace FrameSvgHelpers
{

/**
 * @returns the element id name for said @p borders
 */
QString borderToElementId(FrameSvg::EnabledBorders borders)
{
    switch(borders) {
        case FrameSvg::NoBorder:
            return QStringLiteral("center");
        case FrameSvg::TopBorder:
            return QStringLiteral("top");
        case FrameSvg::BottomBorder:
            return QStringLiteral("bottom");
        case FrameSvg::LeftBorder:
            return QStringLiteral("left");
        case FrameSvg::RightBorder:
            return QStringLiteral("right");
        case FrameSvg::TopBorder | FrameSvg::LeftBorder:
            return QStringLiteral("topleft");
        case FrameSvg::TopBorder | FrameSvg::RightBorder:
            return QStringLiteral("topright");
        case FrameSvg::BottomBorder | FrameSvg::LeftBorder:
            return QStringLiteral("bottomleft");
        case FrameSvg::BottomBorder | FrameSvg::RightBorder:
            return QStringLiteral("bottomright");
        default:
            qWarning() << "unrecognized border" << borders;
    }
    return QString();
}

/**
 * @returns the suggested geometry for the @p borders given a @p fullSize frame size and a @p contentRect
 */
QRect sectionRect(Plasma::FrameSvg::EnabledBorders borders, const QRect& contentRect, const QSize& fullSize)
{
    //don't use QRect corner methods here, they have semantics that might come as unexpected.
    //prefer constructing the points explicitly. e.g. from QRect::topRight docs:
    //Note that for historical reasons this function returns QPoint(left() + width() -1, top()).

    switch(borders) {
        case FrameSvg::NoBorder:
            return contentRect;
        case FrameSvg::TopBorder:
            return QRect(QPoint(contentRect.left(), 0), QSize(contentRect.width(), contentRect.top()));
        case FrameSvg::BottomBorder:
            return QRect(QPoint(contentRect.left(), contentRect.bottom()+1), QSize(contentRect.width(), fullSize.height()-contentRect.bottom()-1));
        case FrameSvg::LeftBorder:
            return QRect(QPoint(0, contentRect.top()), QSize(contentRect.left(), contentRect.height()));
        case FrameSvg::RightBorder:
            return QRect(QPoint(contentRect.right()+1, contentRect.top()), QSize(fullSize.width()-contentRect.right()-1, contentRect.height()));
        case FrameSvg::TopBorder | FrameSvg::LeftBorder:
            return QRect(QPoint(0, 0), QSize(contentRect.left(), contentRect.top()));
        case FrameSvg::TopBorder | FrameSvg::RightBorder:
            return QRect(QPoint(contentRect.right()+1, 0), QSize(fullSize.width()-contentRect.right()-1, contentRect.top()));
        case FrameSvg::BottomBorder | FrameSvg::LeftBorder:
            return QRect(QPoint(0, contentRect.bottom()+1), QSize(contentRect.left(), fullSize.height()-contentRect.bottom()-1));
        case FrameSvg::BottomBorder | FrameSvg::RightBorder:
            return QRect(QPoint(contentRect.right()+1, contentRect.bottom()+1), QSize(fullSize.width()-contentRect.right()-1, fullSize.height()-contentRect.bottom()-1));
        default:
            qWarning() << "unrecognized border" << borders;
            return QRect();
    }
    return QRect();
}

}

}

#endif
