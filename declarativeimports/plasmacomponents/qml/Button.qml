/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2011 by Mark Gaiser <markg85@gmail.com>
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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: button

    // Commmon API
    property bool checked: false
    property bool checkable: false
    property alias pressed: mouse.pressed
    property alias text: label.text
    property alias iconSource: icon.source
    property alias font: label.font

    signal clicked()

    width: Math.max(50, icon.width + label.paintedWidth + surfaceNormal.margins.left + surfaceNormal.margins.right) + ((icon.valid) ? surfaceNormal.margins.left : 0);
    height: Math.max(20, Math.max(icon.height, label.paintedHeight) + surfaceNormal.margins.top + surfaceNormal.margins.bottom)

    // TODO: needs to define if there will be specific graphics for
    //     disabled buttons
    opacity: enabled ? 1.0 : 0.5

    function pressButton() {
        if(button.enabled) {
            buttonContent.state = "pressed"
        }
    }

    function releaseButton() {
        if(button.enabled) {
            buttonContent.state = "normal"

            if (button.checkable) {
                button.checked = !button.checked;
            }

            // TODO: "checked" state must have special graphics?

            button.clicked();
            button.forceActiveFocus();
        }
    }

    Keys.onSpacePressed: pressButton()
    Keys.onReturnPressed: pressButton()
    Keys.onReleased: {
        if (event.key == Qt.Key_Space ||
            event.key == Qt.Key_Return)
            releaseButton();
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            shadow.state = "focus"
        } else if (checked) {
            shadow.state = "hidden"
        } else {
            shadow.state = "shadow"
        }
    }

    ButtonShadow {
        id: shadow
        anchors.fill: parent
    }

    // The normal button state
    PlasmaCore.FrameSvgItem {
        id: surfaceNormal

        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: "normal"
    }

    // The pressed state
    PlasmaCore.FrameSvgItem {
        id: surfacePressed

        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: "pressed"
        opacity: 0
    }

    Item {
        id: buttonContent

        states: [
            State { name: "normal" },
            State { name: "pressed" }
        ]
        transitions: [
            Transition {
                to: "normal"
                // Cross fade from pressed to normal
                ParallelAnimation {
                    NumberAnimation { target: surfaceNormal; property: "opacity"; to: 1; duration: 100 }
                    NumberAnimation { target: surfacePressed; property: "opacity"; to: 0; duration: 100 }
                }
            },
            Transition {
                to: "pressed"
                // Cross fade from normal to pressed
                ParallelAnimation {
                    NumberAnimation { target: surfaceNormal; property: "opacity"; to: 0; duration: 100 }
                    NumberAnimation { target: surfacePressed; property: "opacity"; to: 1; duration: 100 }
                }
            }
        ]

        anchors {
            fill: parent
            leftMargin: surfaceNormal.margins.left
            topMargin: surfaceNormal.margins.top
            rightMargin: surfaceNormal.margins.right
            bottomMargin: surfaceNormal.margins.bottom
        }

        IconLoader {
            id: icon

            anchors {
                verticalCenter: parent.verticalCenter
                left: label.text ? parent.left : undefined
                horizontalCenter: label.text ? undefined : parent.horizontalCenter
            }
        }

        Text {
            id: label

            anchors {
                top: parent.top
                bottom: parent.bottom
                right: parent.right
                left: icon.valid ? icon.right : parent.left
                leftMargin: icon.valid ? parent.anchors.leftMargin : 0
            }

            font.capitalization: theme.defaultFont.capitalization
            font.family: theme.defaultFont.family
            font.italic: theme.defaultFont.italic
            font.letterSpacing: theme.defaultFont.letterSpacing
            font.pointSize: theme.defaultFont.pointSize
            font.strikeout: theme.defaultFont.strikeout
            font.underline: theme.defaultFont.underline
            font.weight: theme.defaultFont.weight
            font.wordSpacing: theme.defaultFont.wordSpacing
            color: theme.buttonTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: true
        onPressed: pressButton()
        onReleased: releaseButton()
        onEntered: {
            shadow.state = "hover"
        }
        onExited: {
            if (button.activeFocus) {
                shadow.state = "focus"
            } else if (checked) {
                shadow.state = "hidden"
            } else {
                shadow.state = "shadow"
            }
        }
    }
}