/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.10 as Kirigami

T.ScrollBar {
    id: controlRoot

    implicitWidth: background.implicitWidth
    implicitHeight: background.implicitHeight

    hoverEnabled: !Kirigami.Settings.isMobile

    visible: controlRoot.size < 1.0

    interactive: !Kirigami.Settings.tabletMode

    background: Item {
        visible: controlRoot.size < 1.0 && controlRoot.interactive
        implicitWidth: scrollbarSvg.elementSize("hint-scrollbar-size").width 
        implicitHeight: implicitWidth
        Rectangle {
            anchors {
                top: parent.top
                bottom: parent.bottom
                left: parent.left
            }
            // the separator line doesn't work yet with the plasmoids design
            //visible: typeof plasmoid === "undefined"
            width: units.devicePixelRatio
            color: PlasmaCore.ColorScope.textColor
            opacity: 0.1
        }
        PlasmaCore.FrameSvgItem {
            anchors.fill: parent
            imagePath:"widgets/scrollbar"
            colorGroup: PlasmaCore.ColorScope.colorGroup

            prefix: controlRoot.horizontal ? "background-horizontal" : "background-vertical"
            opacity: controlRoot.hovered
            Behavior on opacity {
                OpacityAnimator {
                    duration: units.longDuration
                }
            }
        }
    }

    onPositionChanged: {
        disappearTimer.restart();
        handleGraphics.handleState = Math.min(1, handleGraphics.handleState + 0.1)
    }

    contentItem: Item {
        PlasmaCore.FrameSvgItem {
            anchors.fill: parent
            imagePath:"widgets/scrollbar"

            implicitWidth: scrollbarSvg.elementSize("hint-scrollbar-size").width 
            visible: controlRoot.interactive && controlRoot.size < 1.0
            implicitHeight: implicitWidth
            colorGroup: PlasmaCore.ColorScope.colorGroup

            prefix: controlRoot.hovered ? "mouseover-slider" : "slider"
        }
        Rectangle {
            id: handleGraphics

            property real handleState: 0

            visible: !controlRoot.interactive

            x: Math.round(controlRoot.orientation == Qt.Vertical
                ? (Qt.application.layoutDirection === Qt.LeftToRight
                    ? (parent.width - width) - (parent.width/2 - width/2) * handleState
                    : (parent.width/2 - width/2) * handleState)
                : 0)
            
            y: Math.round(controlRoot.orientation == Qt.Horizontal
                ? (parent.height - height) - (parent.height/2 - height/2) * handleState
                : 0)

            NumberAnimation {
                id: resetAnim
                target: handleGraphics
                property: "handleState"
                from: handleGraphics.handleState
                to: 0
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }

            width: Math.round(controlRoot.orientation == Qt.Vertical
                    ? Math.max(2, Kirigami.Units.smallSpacing * handleState)
                    : parent.width)
            height: Math.round(controlRoot.orientation == Qt.Horizontal
                    ? Math.max(2, Kirigami.Units.smallSpacing * handleState)
                    : parent.height)
            radius: Math.min(width, height)
            color: PlasmaCore.ColorScope.textColor
            opacity: 0.3
            Timer {
                id: disappearTimer
                interval: 1000
                onTriggered: {
                    resetAnim.restart();
                    handleGraphics.handleState = 0;
                }
            }
        }
    }

    PlasmaCore.Svg {
        id: scrollbarSvg
        imagePath: "widgets/scrollbar"
        //TODO: support arrows?
        property bool arrowPresent: scrollbarSvg.hasElement("arrow-up")
        //new theme may be different
        onRepaintNeeded: arrowPresent = scrollbarSvg.hasElement("arrow-up")
    }
}
