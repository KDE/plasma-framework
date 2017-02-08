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

T.Drawer {
    id: control

    parent: T.ApplicationWindow.overlay

    implicitWidth: Math.max(background ? background.implicitWidth : 0, contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0, contentHeight + topPadding + bottomPadding)

    contentWidth: contentItem.implicitWidth || (contentChildren.length === 1 ? contentChildren[0].implicitWidth : 0)
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    topPadding: control.edge === Qt.BottomEdge ? 1 : 0
    leftPadding: control.edge === Qt.RightEdge ? 1 : 0
    rightPadding: control.edge === Qt.LeftEdge ? 1 : 0
    bottomPadding: control.edge === Qt.TopEdge ? 1 : 0

    background: PlasmaCore.FrameSvgItem {
        anchors {
            fill: parent
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }
        implicitWidth: units.gridUnit * 12
        imagePath: "widgets/background"
        enabledBorders: {
            switch (control.edge) {
            case Qt.BottomEdge:
                return PlasmaCore.FrameSvgItem.TopBorder;
            case Qt.RightEdge:
                return PlasmaCore.FrameSvgItem.LeftBorder;
            case Qt.TopEdge:
                return PlasmaCore.FrameSvgItem.BottomBorder;
            case Qt.LeftEdge:
            default:
                return PlasmaCore.FrameSvgItem.RightBorder;
            }
        }
    }

    enter: Transition {
        SmoothedAnimation {
            velocity: 5
        }
    }
    exit: Transition {
        SmoothedAnimation {
            velocity: 5
        }
    }
}
