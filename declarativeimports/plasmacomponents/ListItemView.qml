/*
 *   Copyright 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
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
import QtQuick 1.1

ListView {
    id: list

    // Plasma API
    property alias scrollVisible: scrollBar.visible

    // Convinience API
    property bool __isVertical: orientation == Qt.Vertical

    orientation: Qt.Vertical

    highlightFollowsCurrentItem: true
    highlight: ListHighlight {
        width: ListView.view.width
        height: ListView.view.currentItem.height
        pressed: ListView.view.currentItem.pressed
        hover: ListView.view.currentItem.hover
    }

    ScrollBar {
        id: scrollBar

        anchors {
            top: __isVertical ? parent.top : undefined
            left: __isVertical ? undefined : parent.left
            right: parent.right
            bottom: parent.bottom
        }
        animated: true
        stepSize: highlightItem.height
        orientation: parent.orientation
        flickableItem: parent
    }
}