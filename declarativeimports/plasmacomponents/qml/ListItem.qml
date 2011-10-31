/*
 *   Copyright 2010 Marco Martin <notmart@gmail.com>
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
    id: listItem
    default property alias content: paddingItem.data

    //this defines if the item will emit clicked and look pressed on mouse down
    property bool enabled: false
    //item has been clicked or pressed+hold
    signal clicked
    signal pressAndHold

    //plasma extension
    //always look pressed?
    property bool checked: false
    //is this to be used as section delegate?
    property bool sectionDelegate: false

    width: parent.width
    height: paddingItem.childrenRect.height + background.margins.top + background.margins.bottom

    property int implicitHeight: paddingItem.childrenRect.height + background.margins.top + background.margins.bottom


    Connections {
        target: listItem
        onCheckedChanged: background.prefix = (listItem.checked ? "pressed" : "normal")
        onSectionDelegateChanged: background.prefix = (listItem.sectionDelegate ? "section" : "normal")
    }
    PlasmaCore.FrameSvgItem {
        id : background
        imagePath: "widgets/listitem"
        prefix: "normal"

        anchors.fill: parent
        Component.onCompleted: {
            prefix = (listItem.sectionDelegate ? "section" : (listItem.checked ? "pressed" : "normal"))
        }
    }
    PlasmaCore.SvgItem {
        svg: PlasmaCore.Svg {imagePath: "widgets/listitem"}
        elementId: "separator"
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: naturalSize.height
        visible: listItem.sectionDelegate || (index != undefined && index > 0 && !listItem.checked && !itemMouse.pressed)
    }

    MouseArea {
        id: itemMouse
        property bool changeBackgroundOnPress: !listItem.checked && !listItem.sectionDelegate
        anchors.fill: background
        enabled: listItem.enabled

        onClicked: listItem.clicked()
        onPressAndHold: listItem.pressAndHold()
        onPressed: if (changeBackgroundOnPress) background.prefix = "pressed"
        onReleased: if (changeBackgroundOnPress) background.prefix = "normal"
        onCanceled: if (changeBackgroundOnPress) background.prefix = "normal"
    }

    Item {
        id: paddingItem
        anchors {
            fill: background
            leftMargin: background.margins.left
            topMargin: background.margins.top
            rightMargin: background.margins.right
            bottomMargin: background.margins.bottom
        }
    }
}
