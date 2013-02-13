/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0

import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents

Rectangle {
    id: root
    color: "darkblue"
    width: 640
    height: 480

    property Item toolBox

    Connections {
        target: plasmoid
        onAppletAdded: {
            var container = appletContainerComponent.createObject(root)
            container.visible = true
            print("Applet added: " + applet)
            applet.parent = container
            container.applet = applet
            applet.anchors.fill= applet.parent
            applet.visible = true
        }
    }

    PlasmaCore.Svg {
        id: actionssvg
        imagePath: "widgets/configuration-icons"
    }

    Component {
        id: appletContainerComponent
        PlasmaCore.FrameSvgItem {
            id: frame
            x: 50
            y: 50
            width: large
            height: large

            property alias applet: appletContainer.children
            property int small: 90
            property int large: root.width /2
            imagePath: applet[0].backgroundHints == 0 ? "" : "widgets/background"
            MouseArea {
                anchors.fill: parent
                drag.target: parent
                onClicked: {
                    var s = (frame.width == frame.large) ? frame.small : frame.large;
                    frame.x = s
                    frame.height = s
                    frame.width = s
                }
            }
            Behavior on x { PropertyAnimation { easing.type: Easing.OutElastic; duration: 800 } }
            //Behavior on y { PropertyAnimation { easing.type: Easing.OutElastic; duration: 800 } }
            Behavior on width { PropertyAnimation { easing.type: Easing.InOutDouble; duration: 300 } }
            Behavior on height { PropertyAnimation { easing.type: Easing.InOutDouble; duration: 300 } }

            Item {
                id: appletContainer
                anchors {
                    fill: parent
                    leftMargin: frame.margins.left
                    rightMargin: parent.margins.right
                    topMargin: parent.margins.top
                    bottomMargin: parent.margins.bottom
                }
            }
            PlasmaCore.SvgItem {
                svg: actionssvg
                elementId: "rotate"
                width: 16
                height: width
                anchors.margins: frame.margins.left
                anchors {
                    top: parent.top
                    left: parent.left
                }
                //Rectangle { color: "white"; opacity: 0.2; anchors.fill: parent; }
            }
            PlasmaComponents.BusyIndicator {
                z: 1000
                visible: applet[0].busy
                running: visible
                anchors.centerIn: parent
            }
        }
    }

    PlasmaCore.IconItem {
        source: "accessories-dictionary"
        x: 50
        y: 350
        width: 48
        height: 48
        Rectangle { color: "white"; opacity: 0.2; anchors.fill: parent; }
    }

    Component.onCompleted: {
        print("Test Containment loaded")
        print(plasmoid)
    }
}