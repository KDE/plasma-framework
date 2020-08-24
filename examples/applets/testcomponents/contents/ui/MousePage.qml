/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

// MousePage

PlasmaComponents.Page {
    id: mousePage
    anchors {
        fill: parent
        margins: _s
    }
    PlasmaExtras.Heading {
        id: mellabel
        level: 1
        text: "MouseEventListener"
        anchors { left: parent.left; right: parent.right; top: parent.top }
    }
    KQuickControlsAddons.MouseEventListener {
        id: mel
        hoverEnabled: true
        anchors { left: parent.left; right: parent.right; top: mellabel.bottom; bottom: parent.bottom; }
        /*
        void pressed(KDeclarativeMouseEvent *mouse);
        void positionChanged(KDeclarativeMouseEvent *mouse);
        void released(KDeclarativeMouseEvent *mouse);
        void clicked(KDeclarativeMouseEvent *mouse);
        void pressAndHold(KDeclarativeMouseEvent *mouse);
        void wheelMoved(KDeclarativeWheelEvent *wheel);
        void containsMouseChanged(bool containsMouseChanged);
        void hoverEnabledChanged(bool hoverEnabled);
        */
        onPressed: {
            print("Pressed");
            melstatus.text = "pressed";
        }
        onPositionChanged: {
            print("positionChanged: " + mouse.x + "," + mouse.y);
        }
        onReleased: {
            print("Released");
            melstatus.text = "Released";
        }
        onPressAndHold: {
            print("pressAndHold");
            melstatus.text = "pressAndHold";
        }
        onClicked: {
            print("Clicked");
            melstatus.text = "clicked";
        }
        onWheelMoved: {
            print("Wheel: " + wheel.delta);
        }
        onContainsMouseChanged: {
            print("Contains mouse: " + containsMouse);
        }

        MouseArea {
            //target: mel
            anchors.fill: parent
            onPressed: PlasmaExtras.DisappearAnimation { targetItem: bgImage }
            onReleased: PlasmaExtras.AppearAnimation { targetItem: bgImage }
        }
        Image {
            id: bgImage
            source: "image://appbackgrounds/standard"
            fillMode: Image.Tile
            anchors.fill: parent
            asynchronous: true
//                 opacity: mel.containsMouse ? 1 : 0.2
//                 Behavior on opacity { PropertyAnimation {} }
        }
        Column {
            //width: parent.width
            spacing: _s
            anchors.fill: parent
            PlasmaComponents.Button {
                text: "Button"
                iconSource: "call-start"
            }
            PlasmaComponents.ToolButton {
                text: "ToolButton"
                iconSource: "call-stop"
            }
            PlasmaComponents.RadioButton {
                text: "RadioButton"
                //iconSource: "call-stop"
            }
            PlasmaComponents.Label {
                id: melstatus
            }
        }

    }
}

