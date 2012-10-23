/*
*   Copyright 2012 Marco Martin <mart@kde.org>
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

/**Documented API
Inherits:
        Item

Imports:
        QtQuick 1.1
        org.kde.plasma.components 0.1

Description:
    This item takes a Flickable and automatically puts scrollbars in adjusting the layout if needed. The scrollbars will be interactive or not, depending on the platform.
    If flickableItem is a categorized ListView the vertical scrollbar will be a SectionScroller.

Properties:
    Item flickableItem:
    The Flickable of this area: it can be either a Flickable or a subclass, ListView or GridView
**/

import QtQuick 1.1
import org.kde.plasma.components 0.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: root

    property Flickable flickableItem
    //FIXME: this alias seems necessary for it to correctly parse
    default property alias flickableItemDefault: root.flickableItem
    clip: true

    Connections {
        target: root
        onFlickableItemChanged: {
            root.flickableItem.parent = root
            root.flickableItem.anchors.fill = root
            internal.checkVerticalScrollBar()
            internal.checkHorizontalScrollBar()
        }
    }
    Component.onCompleted: {
        root.flickableItem.parent = root
        root.flickableItem.anchors.fill = root
        internal.checkVerticalScrollBar()
        internal.checkHorizontalScrollBar()
    }
    Connections {
        target: flickableItem
        onContentHeightChanged: internal.checkVerticalScrollBar()
        onHeightChanged: internal.checkVerticalScrollBar()
        onContentWidthChanged: internal.checkHorizontalScrollBar()
        onWidthChanged: internal.checkHorizontalScrollBar()
    }
    QtObject {
        id: internal
        property Item verticalScrollBar
        property Item horizontalScrollBar

        function checkVerticalScrollBar()
        {
            if (!flickableItem) {
                return
            }

            if (flickableItem.contentHeight > flickableItem.height) {
                //Do we have to change the type?
                //from section to normal
                if ((!flickableItem.model || flickableItem.model.get === undefined || !flickableItem.section || !flickableItem.section.property) &&
                    (!verticalScrollBar || verticalScrollBar.orientation === undefined)) {
                    if (verticalScrollBar) verticalScrollBar.destroy()
                    verticalScrollBar = verticalScrollBarComponent.createObject(root)
                //from normal to section
                } else if (flickableItem.section && flickableItem.section.property &&
                    flickableItem.model.get !== undefined &&
                    (!verticalScrollBar || verticalScrollBar.orientation !== undefined)) {
                    if (verticalScrollBar) verticalScrollBar.destroy()
                    verticalScrollBar = sectionScrollerComponent.createObject(root)
                }
            }

            //undefined in case of SectionScroller
            if ((flickableItem.contentHeight > flickableItem.height) &&
                (verticalScrollBar.interactive || (verticalScrollBar.orientation === undefined &&
                //FIXME: heuristic on width to distinguish the touch sectionscroller
                verticalScrollBar.width < 30))) {
                flickableItem.anchors.rightMargin = verticalScrollBar.width
            } else {
                flickableItem.anchors.rightMargin = 0
            }
        }

        function checkHorizontalScrollBar()
        {
            if (!flickableItem || horizontalScrollBar) {
                return
            }

            if (flickableItem.contentWidth > flickableItem.width) {
                if (!horizontalScrollBar) {
                    horizontalScrollBar = horizontalScrollBarComponent.createObject(root)
                }
            }

            if ((flickableItem.contentWidth > flickableItem.width) &&
                horizontalScrollBar.interactive) {
                flickableItem.anchors.bottomMargin = horizontalScrollBar.height
            } else {
                flickableItem.anchors.bottomMargin = 0
            }
        }
    }
    Component {
        id: verticalScrollBarComponent
        ScrollBar {
            flickableItem: root.flickableItem
            orientation: Qt.Vertical
            property bool isScrollBar: true
            anchors {
                left: undefined
                top: parent.top
                right: parent.right
                bottom: parent.bottom
            }
        }
    }
    Component {
        id: horizontalScrollBarComponent
        ScrollBar {
            flickableItem: root.flickableItem
            orientation: Qt.Horizontal
            anchors {
                left: parent.left
                top: undefined
                right: parent.right
                bottom: parent.bottom
            }
        }
    }
    Component {
        id: sectionScrollerComponent
        SectionScroller {
            listView: root.flickableItem
            property bool isScrollBar: false
            anchors {
                left: undefined
                top: parent.top
                right: parent.right
                bottom: parent.bottom
            }
        }
    }
    //FIXME: create all this stuff only on demand, like scrollbars?
    PlasmaCore.Svg {
        id: borderSvg
        imagePath: "widgets/scrollwidget"
    }
    PlasmaCore.SvgItem {
        svg: borderSvg
        z: 1000
        elementId: "border-top"
        width: 100
        height: naturalSize.height
        opacity: flickableItem.atYBeginning ? 0 : 1
        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }
        anchors {
            left: parent.left
            top: parent.top
            right: parent.right
        }
    }
    PlasmaCore.SvgItem {
        svg: borderSvg
        z: 1000
        elementId: "border-bottom"
        width: 100
        height: naturalSize.height
        opacity: flickableItem.atYEnd ? 0 : 1
        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }
        anchors {
            left: parent.left
            bottom: parent.bottom
            right: parent.right
        }
    }
    PlasmaCore.SvgItem {
        svg: borderSvg
        z: 1000
        elementId: "border-left"
        width: naturalSize.width
        opacity: flickableItem.atXBeginning ? 0 : 1
        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }
        anchors {
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
    }
    PlasmaCore.SvgItem {
        svg: borderSvg
        z: 1000
        elementId: "border-right"
        width: naturalSize.width
        opacity: flickableItem.atXEnd ? 0 : 1
        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }
        anchors {
            top: parent.top
            bottom: parent.bottom
            right: parent.right
        }
    }
}
