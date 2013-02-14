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
import org.kde.qtextracomponents 0.1 as QtExtras

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
            x: 50 - tmargin
            y: 50 - lmargin
            width: large + lmargin + rmargin
            height: large + tmargin + bmargin

            property alias applet: appletContainer.children
            property int small: 90
            property int large: 400

            property int lmargin: imagePath != "" ? frame.margins.left : 0
            property int rmargin: imagePath != "" ? frame.margins.right : 0
            property int tmargin: imagePath != "" ? frame.margins.top : 0
            property int bmargin: imagePath != "" ? frame.margins.bottom : 0

            imagePath: applet.length > 0 && applet[0].backgroundHints == 0 ? "" : "widgets/background"
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
                visible: applet.length > 0 && applet[0].busy
                running: visible
                anchors.centerIn: parent
            }
        }
    }

    QtExtras.QIconItem {
        icon: "preferences-desktop-icons"
        width: 96
        height: width
        MouseArea {
            anchors.fill: parent
            drag.target: parent
            onClicked: {
                var n = parent.width == 96 ? 256 : 96;
                parent.width = n;
                parent.height = n;
            }
        }
        Behavior on width { PropertyAnimation { easing.type: Easing.InOutDouble; duration: 100 } }
        Behavior on height { PropertyAnimation { easing.type: Easing.InOutDouble; duration: 100 } }
    }

    PlasmaCore.IconItem {
        source: "accessories-dictionary"
        x: 50
        y: 350
        width: 48
        height: 48
        //Rectangle { color: "white"; opacity: 0.2; anchors.fill: parent; }
        MouseArea {
            anchors.fill: parent
            drag.target: parent
        }

    }

    Component.onCompleted: {
        print("Test Containment loaded")
        print(plasmoid)
    }
}