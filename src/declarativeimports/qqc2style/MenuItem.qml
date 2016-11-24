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
import QtQuick.Templates 2.0 as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.MenuItem {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             Math.max(contentItem.implicitHeight,
                                      indicator ? indicator.implicitHeight : 0) + topPadding + bottomPadding)
    baselineOffset: contentItem.y + contentItem.baselineOffset

    leftPadding: highlight.margins.left
    topPadding: highlight.margins.top
    rightPadding: highlight.margins.right
    bottomPadding: highlight.margins.bottom
    spacing: units.smallSpacing
    hoverEnabled: true

    contentItem: Label {
        leftPadding: !control.mirrored ? (control.indicator ? control.indicator.width : 0) + control.spacing : 0
        rightPadding: control.mirrored ? (control.indicator ? control.indicator.width : 0) + control.spacing : 0

        text: control.text
        font: control.font
        color: theme.textColor
        elide: Text.ElideRight
        visible: control.text
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    indicator: CheckIndicator {
        x: control.mirrored ? control.width - width - control.rightPadding : control.leftPadding
        y: control.topPadding + (control.availableHeight - height) / 2

        visible: control.checkable
        checkState: control.checked ? Qt.Checked : 0
    }

    background: Item {
        implicitWidth: units.gridUnit * 8

        PlasmaCore.FrameSvgItem {
            id: highlight
            imagePath: "widgets/viewitem"
            prefix: "hover"
            colorGroup: PlasmaCore.ColorScope.colorGroup
            anchors.fill: parent
            opacity: control.hovered && !control.pressed ? 1 : 0
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                }
            }
        }
    }
}
