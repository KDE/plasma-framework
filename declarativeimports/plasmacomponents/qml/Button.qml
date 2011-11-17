/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2011 by Mark Gaiser <markg85@gmail.com>
*   Copyright (C) 2011 by Marco Martin <mart@kde.org>
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

import QtQuick 1.1
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

    implicitWidth: {
        if (label.paintedWidth == 0) {
            return height
        } else {
            //return Math.max(theme.defaultFont.mSize.width*12, label.paintedWidth)
            return Math.max(theme.defaultFont.mSize.width*12, icon.width + label.paintedWidth + surfaceNormal.margins.left + surfaceNormal.margins.right) + ((icon.valid) ? surfaceNormal.margins.left : 0)
        }
    }
    implicitHeight: Math.max(theme.defaultFont.mSize.height*1.8, Math.max(icon.height, label.paintedHeight) + surfaceNormal.margins.top + surfaceNormal.margins.bottom)

    // TODO: needs to define if there will be specific graphics for
    //     disabled buttons
    opacity: enabled ? 1.0 : 0.5

    QtObject {
        id: internal
        property bool userPressed: false

        function belongsToButtonGroup()
        {
            return button.parent
                   && button.parent.hasOwnProperty("checkedButton")
                   && button.parent.exclusive
        }

        function pressButton()
        {
            userPressed = true
        }

        function releaseButton()
        {
            userPressed = false
            if (!button.enabled) {
                return
            }

            if ((!belongsToButtonGroup() || !button.checked) && button.checkable) {
                button.checked = !button.checked
            }

            button.clicked()
            button.forceActiveFocus()
        }
    }

    Keys.onSpacePressed: internal.pressButton()
    Keys.onReturnPressed: internal.pressButton()
    Keys.onReleased: {
        if (event.key == Qt.Key_Space ||
            event.key == Qt.Key_Return)
            internal.releaseButton();
    }

    ButtonShadow {
        id: shadow
        anchors.fill: parent
        state: {
            if (internal.userPressed || checked) {
                return "hidden"
            } else if (mouse.containsMouse) {
                return "hover"
            } else if (button.activeFocus) {
                return "focus"
            } else {
                return "shadow"
            }
        }
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
        state: (internal.userPressed || checked) ? "pressed" : "normal"

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
                left: label.paintedWidth > 0 ? parent.left : undefined
                horizontalCenter: label.paintedWidth > 0 ? undefined : parent.horizontalCenter
            }
        }

        Text {
            id: label

            //FIXME: why this is needed?
            onPaintedWidthChanged: {
                icon.anchors.horizontalCenter = label.paintedWidth > 0 ? undefined : icon.parent.horizontalCenter
                icon.anchors.left = label.paintedWidth > 0 ? icon.parent.left : undefined
            }

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
            horizontalAlignment: icon.valid ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: true
        onPressed: internal.pressButton()
        onReleased: internal.releaseButton()
        onCanceled: internal.releaseButton()
    }
}