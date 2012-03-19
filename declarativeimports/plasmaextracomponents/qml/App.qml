/*
*   Copyright 2012 by Sebastian Kügler <sebas@kde.org>
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
    org.kde.plasma.extras

Description:
    This is a container for Apps. Navigation and Content area layout can
    vary depending on the available size.

Properties:
    Item navigation: Navigation or Context Area.
    Item content: The "main view"

Methods:
    

Signals:
    
**/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
//import org.kde.plasma.extras 0.1 as PlasmaExtras

Item {
    id: app
    property alias navigation: navigationItem.children
    property alias content: contentItem.children
    property int space: 12
    property int navigationWidth: 340
    property int contentWidth: app.width - navigationWidth
    anchors.fill: parent

    PlasmaCore.Theme { id: theme }

    Image {
        id: contentBackground
        anchors { left: navigationBackground.right; right: parent.right; top: parent.top; bottom: parent.bottom; }
        source: "image://appbackgrounds/standard"
        fillMode: Image.Tile
        asynchronous: true
        width: childrenRect.width
        Item {
            id: contentItem
            width: app.width - navigationWidth - space*2
            height: app.height
            anchors { left: parent.left; leftMargin: space; rightMargin: space; top: parent.top; bottom: parent.bottom; right: parent.right; }
        }

    }
    Image {
        id: navigationBackground
        anchors { left: parent.left; top: parent.top; bottom: parent.bottom; }
        source: "image://appbackgrounds/contextarea"
        fillMode: Image.Tile
        asynchronous: true
        clip: true
        width: navigationWidth + space
        Item {
            id: navigationItem
            anchors.fill: parent
        }
    }
    Image {
        source: "image://appbackgrounds/shadow-right"
        fillMode: Image.Tile
        anchors {
            left: navigationBackground.right
            top: parent.top
            bottom: parent.bottom
            leftMargin: -1
        }
    }

    onNavigationWidthChanged: {
        navigationBackground.width = navigationWidth;
        navigationItem.width = navigationWidth;
    }
}
