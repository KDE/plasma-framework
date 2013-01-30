/*
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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import "." 0.1

Item {
    id: root

    default property alias content: menuColumn.children
    property Item visualParent
    property int status: DialogStatus.Closed

    onVisualParentChanged: {
        //if is a menuitem move to menuColumn
        if (visualParent.separator !== undefined) {
            var obj = arrowComponent.createObject(visualParent)
        }
    }

    Component {
        id: arrowComponent
        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {imagePath: "widgets/arrows"}
            elementId: "right-arrow"
            width: naturalSize.width
            height: naturalSize.height
            anchors {
                right: parent.right
                verticalCenter: parent.verticalCenter
            }
        }
    }
    function open()
    {
        var parent = root.visualParent ? root.visualParent : root.parent
        var pos = dialog.popupPosition(parent, Qt.AlignCenter)
        dialog.x = pos.x
        dialog.y = pos.y

        dialog.visible = true
        dialog.activateWindow()
    }

    function close()
    {
        dialog.visible = false
    }

    function addMenuItem(item)
    {
        item.parent = menuColumn
    }

    onChildrenChanged: {
        for (var i = 0; i < children.length; ++i) {
            var item = children[i]
            //if is a menuitem move to menuColumn
            if (item.separator !== undefined) {
                item.parent = menuColumn
            }
        }
    }

    visible: false

    PlasmaCore.Dialog {
        id: dialog
        visible: false
        windowFlags: Qt.Popup
        onVisibleChanged: {
            if (visible) {
                status = DialogStatus.Open
            } else {
                status = DialogStatus.Closed
            }
        }

        mainItem: Item {
            id: contentItem

            width: Math.max(menuColumn.width, theme.defaultFont.mSize.width * 12)
            height: Math.min(menuColumn.height, theme.defaultFont.mSize.height * 25)



            Flickable {
                id: listView
                anchors.fill: parent

                clip: true

                Column {
                    id: menuColumn
                    spacing: 4
                    onChildrenChanged: {
                        for (var i = 0; i < children.length; ++i) {
                            if (children[i].clicked != undefined)
                                children[i].clicked.connect(root.close)
                        }
                    }
                }
            }

            ScrollBar {
                id: scrollBar
                flickableItem: listView
                visible: listView.contentHeight > contentItem.height
                //platformInverted: root.platformInverted
                anchors { top: listView.top; right: listView.right }
            }
        }
    }


    onStatusChanged: {
        if (status == DialogStatus.Opening) {
            if (listView.currentItem != null) {
                listView.currentItem.focus = false
            }
            listView.currentIndex = -1
            listView.positionViewAtIndex(0, ListView.Beginning)
        }
        else if (status == DialogStatus.Open) {
            listView.focus = true
        }
    }
}
