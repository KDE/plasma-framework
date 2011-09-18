/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
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

    // Plasma API
    property QtObject theme: PlasmaCore.Theme { }

    function pressButton() {
        if (button.enabled)
            surface.prefix = "pressed";
    }

    function releaseButton() {
        if (!button.enabled)
            return;

        if (button.checkable)
            button.checked = !button.checked;

        // TODO: "checked" state must have special graphics?
        if (button.checked)
            surface.prefix = "pressed";
        else
            surface.prefix = "normal";

        button.clicked();
        button.forceActiveFocus();
    }

    width: 50
    height: 20
    // TODO: needs to define if there will be specific graphics for
    //     disabled buttons
    opacity: enabled ? 1.0 : 0.5

    Keys.onSpacePressed: pressButton();
    Keys.onReturnPressed: pressButton();
    Keys.onReleased: {
        if (event.key == Qt.Key_Space ||
            event.key == Qt.Key_Return)
            releaseButton();
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            shadow.opacity = 0;
            hover.opacity = 1;
        }else {
            shadow.opacity = 1;
            hover.opacity = 0;
        }
    }

    PlasmaCore.FrameSvgItem {
        id: hover

        anchors.fill: parent
        opacity: 0
        anchors.leftMargin: -margins.left
        anchors.topMargin: -margins.top
        anchors.rightMargin: -margins.right
        anchors.bottomMargin: -margins.bottom
        imagePath: "widgets/button"
        prefix: "hover"

        Behavior on opacity {
            PropertyAnimation { duration: 250 }
        }
    }

    PlasmaCore.FrameSvgItem {
        id: shadow

        anchors {
            fill: parent
            leftMargin: -margins.left
            topMargin: -margins.top
            rightMargin: -margins.right
            bottomMargin: -margins.bottom
        }
        imagePath: "widgets/button"
        prefix: "shadow"

        Behavior on opacity {
            PropertyAnimation { duration: 250 }
        }
    }

    PlasmaCore.FrameSvgItem {
        id: surface

        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: "normal"
    }

    Item {
        anchors {
            fill: parent
            leftMargin: surface.margins.left
            topMargin: surface.margins.top
            rightMargin: surface.margins.right
            bottomMargin: surface.margins.bottom
        }

        Image {
            id: icon

            anchors {
                fill: label.text ? undefined : parent
                top: label.text ? parent.top : undefined
                left: label.text ? parent.left : undefined
                bottom: label.text ? parent.bottom : undefined
            }
            fillMode: Image.PreserveAspectFit
        }

        Text {
            id: label

            anchors {
                top: parent.top
                bottom: parent.bottom
                left: icon.right
                right: parent.right
            }
            color: theme.buttonTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: true

        onPressed: {
            pressButton();
        }
        onReleased: {
            releaseButton();
        }
        onEntered: {
            shadow.opacity = 0;
            hover.opacity = 1;
        }
        onExited: {
            if (button.activeFocus) {
                shadow.opacity = 0;
                hover.opacity = 1;
            } else {
                shadow.opacity = 1;
                hover.opacity = 0;
            }
        }
    }
}

