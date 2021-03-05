/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.Drawer {
    id: control

    parent: T.ApplicationWindow.overlay

    implicitWidth: Math.max(background ? background.implicitWidth : 0, contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0, contentHeight + topPadding + bottomPadding)

    contentWidth: contentItem.implicitWidth || (contentChildren.length === 1 ? contentChildren[0].implicitWidth : 0)
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    topPadding: control.edge === Qt.BottomEdge ? Math.round(PlasmaCore.Units.devicePixelRatio) : 0
    leftPadding: control.edge === Qt.RightEdge ? Math.round(PlasmaCore.Units.devicePixelRatio) : 0
    rightPadding: control.edge === Qt.LeftEdge ? Math.round(PlasmaCore.Units.devicePixelRatio) : 0
    bottomPadding: control.edge === Qt.TopEdge ? Math.round(PlasmaCore.Units.devicePixelRatio) : 0

    background: PlasmaCore.FrameSvgItem {
        anchors {
            fill: parent
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }
        implicitWidth: PlasmaCore.Units.gridUnit * 12
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
