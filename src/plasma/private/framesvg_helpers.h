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
    if (borders == FrameSvg::NoBorder) {
        return QStringLiteral("center");
    } else if (borders == FrameSvg::TopBorder) {
        return QStringLiteral("top");
    } else if (borders == FrameSvg::BottomBorder) {
        return QStringLiteral("bottom");
    } else if (borders == FrameSvg::LeftBorder) {
        return QStringLiteral("left");
    } else if (borders == FrameSvg::RightBorder) {
        return QStringLiteral("right");
    } else if (borders == (FrameSvg::TopBorder | FrameSvg::LeftBorder)) {
        return QStringLiteral("topleft");
    } else if (borders == (FrameSvg::TopBorder | FrameSvg::RightBorder)) {
        return QStringLiteral("topright");
    } else if (borders == (FrameSvg::BottomBorder | FrameSvg::LeftBorder)) {
        return QStringLiteral("bottomleft");
    } else if (borders == (FrameSvg::BottomBorder | FrameSvg::RightBorder)) {
        return QStringLiteral("bottomright");
    } else {
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

    if (borders == FrameSvg::NoBorder) {
        return contentRect;
    } else if (borders == FrameSvg::TopBorder) {
        return QRect(QPoint(contentRect.left(), 0), QSize(contentRect.width(), contentRect.top()));
    } else if (borders == FrameSvg::BottomBorder) {
        return QRect(QPoint(contentRect.left(), contentRect.bottom()+1), QSize(contentRect.width(), fullSize.height()-contentRect.bottom()-1));
    } else if (borders == FrameSvg::LeftBorder) {
        return QRect(QPoint(0, contentRect.top()), QSize(contentRect.left(), contentRect.height()));
    } else if (borders == FrameSvg::RightBorder) {
        return QRect(QPoint(contentRect.right()+1, contentRect.top()), QSize(fullSize.width()-contentRect.right()-1, contentRect.height()));
    } else if (borders == (FrameSvg::TopBorder | FrameSvg::LeftBorder)) {
        return QRect(QPoint(0, 0), QSize(contentRect.left(), contentRect.top()));
    } else if (borders == (FrameSvg::TopBorder | FrameSvg::RightBorder)) {
        return QRect(QPoint(contentRect.right()+1, 0), QSize(fullSize.width()-contentRect.right()-1, contentRect.top()));
    } else if (borders == (FrameSvg::BottomBorder | FrameSvg::LeftBorder)) {
        return QRect(QPoint(0, contentRect.bottom()+1), QSize(contentRect.left(), fullSize.height()-contentRect.bottom()-1));
    } else if (borders == (FrameSvg::BottomBorder | FrameSvg::RightBorder)) {
        return QRect(QPoint(contentRect.right()+1, contentRect.bottom()+1), QSize(fullSize.width()-contentRect.right()-1, fullSize.height()-contentRect.bottom()-1));
    } else {
        qWarning() << "unrecognized border" << borders;
    }
    return QRect();
}

}

}

#endif
