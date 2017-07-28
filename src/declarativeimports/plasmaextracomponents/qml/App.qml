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
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

/**
 * This is a container for Apps. Navigation and Content area layout can vary
 * depending on the available size. This class can be used as top level Item or
 * inside your top-level item. It will manage context or navigation area,
 * content area and toolbars in a way consistent with other apps, and adaptive
 * to the device.
 *
 * @remark App is experimental, its API might change.
 * @deprecated: use org.kde.kirigami ApplicationWindow or QtQuick.Controls 2.0 ApplicationWindow
 *
 * Example usage:
 * @code
 * import QtQuick 2.0
 * import org.kde.plasma.core 2.0 as PlasmaCore
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 *
 * PlasmaExtras.App {
 *     id: app
 *     width: 800
 *     height: 600
 *
 *     navigation: navigationItem
 *     content: contentItem
 *
 *     Item {
 *         id: navigationItem
 *         // [...] this is the navigation area
 *     }
 *
 *     Loader {
 *         id: contentItem
 *         source: "gallery/Extras.qml"
 *         parent: contentArea
 *     }
 * }
 * @endcode
 */
Item {
    id: app
    /**
     * type:Item
     * Navigation or Context Area.
     */
    property alias navigation: navigationItem.children

    /**
     * type:Item
     * The "main view"
     */
    property alias content: contentItem.children

    /**
     * type:Item
     * the topBar's ToolBar.tools
     */
    property alias tools: topBar.tools

    property alias contentArea: contentBackground
    property int space: 20
    property int navigationWidth: 340
    property int contentWidth: app.width - navigationWidth - space*2

    PlasmaCore.Theme { id: theme }

    PlasmaComponents.ToolBar {
        id: topBar
        height: visible ? 48 : 0
        //visible: tools.id != "undefined"
        anchors { top: parent.top; left: parent.left; right: parent.right; }
        //tools: PlasmaComponents.ToolBarLayout {}

        onToolsChanged: {
            // FIXME: questionable heuristics
            height = (tools && tools.childrenRect.width > 20) ? tools.childrenRect.height + space : 0
        }
    }

    Image {
        id: contentBackground
        anchors { left: navigationBackground.right; right: parent.right; top: topBar.bottom; bottom: parent.bottom; }
        source: "image://appbackgrounds/standard"
        fillMode: Image.Tile
        asynchronous: true
        //width: contentWidth
        Item {
            id: contentItem
            //width: contentWidth - 100
            height: app.height
            anchors { left: parent.left; margins: space; top: parent.top; bottom: parent.bottom; right: parent.right; }
        }

    }
    Image {
        id: navigationBackground
        anchors { left: parent.left; top: topBar.bottom; bottom: parent.bottom; }
        source: "image://appbackgrounds/contextarea"
        fillMode: Image.Tile
        asynchronous: true
        clip: true
        width: navigationWidth + space
        Behavior on width {
            NumberAnimation { duration: units.longDuration; easing.type: Easing.InOutExpo; }
        }

        Item {
            clip: true
            id: navigationItem
            anchors.fill: parent
        }
    }
    Image {
        source: "image://appbackgrounds/shadow-left"
        fillMode: Image.Tile
        anchors {
            right: navigationBackground.right
            top: topBar.bottom
            bottom: parent.bottom
            leftMargin: -1
        }
    }

    Image {
        source: "image://appbackgrounds/shadow-bottom"
        fillMode: Image.Tile
        anchors {
            right: navigationBackground.right
            left: navigationBackground.left
            top: topBar.bottom
            //bottom: parent.bottom
            //leftMargin: -1
        }
    }

    onContentChanged: {
        print("content changed to ");
        //content.parent = contentBackground;
        //contentItem.width = contentWidth;
        //contentItem.anchors.rightMargin = 100
    }

    onNavigationWidthChanged: {
        navigationBackground.width = navigationWidth;
        navigationItem.width = navigationWidth;
    }
}
