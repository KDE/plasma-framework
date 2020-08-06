/*
 *   Copyright 2016 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.ToolBar {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0, contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0, contentHeight + topPadding + bottomPadding)

    contentWidth: contentChildren[0].implicitWidth
    contentHeight: contentChildren[0].implicitHeight

    padding: units.smallSpacing
    contentItem: Item { }

    background: PlasmaCore.FrameSvgItem {
        implicitHeight: 40
        imagePath: "widgets/toolbar"
        colorGroup: PlasmaCore.ColorScope.colorGroup
        enabledBorders: control.position == T.ToolBar.Header ?  PlasmaCore.FrameSvgItem.BottomBorder : PlasmaCore.FrameSvgItem.TopBorder
        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                imagePath: "widgets/listitem"
            }
            elementId: "separator"
            anchors {
                left: parent.left
                right: parent.right
                top: control.position == T.ToolBar.Footer || (control.parent.footer && control.parent.footer == control) ? parent.top : undefined
                bottom: control.position == T.ToolBar.Footer || (control.parent.footer && control.parent.footer == control) ? undefined : parent.bottom
            }
        }
    }
}
