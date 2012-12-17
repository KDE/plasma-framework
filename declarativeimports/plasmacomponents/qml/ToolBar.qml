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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

/**
 * A plasma theme based toolbar.
 */
Item{
    id: toolBar
    width: parent.width
    height: (tools && enabled) ? tools.height + frameSvg.margins.top + frameSvg.margins.bottom : 0
    visible: height > 0

    /**
     * type:Object margins
     * margins from the toolbar to the contents. It has 4 properties: left,
     * top, right, bottom
     */
    property alias margins: frameSvg.margins

    Behavior on height {
        PropertyAnimation { 
            id: heightAnimation
            duration: 250
        }
    }
    z: 1000

    /**
     * The ToolBarLayout that contains the ToolButton components that are
     * contained in the ToolBar. ToolBarLayout is not mandatory.
     *
     * The default value is null.
     */
    property Item tools

    /**
     * The type of transition to be used for the ToolBar when the page changes
     * on the relevant PageStack.
     *
     * The possible values can be one of the following:
     *
     * - set: an instantaneous change (default)
     * - push: follows page stack push animation
     * - pop: follows page stack pop animation
     * - replace: follows page stack replace animation
     */
    property string transition: "set"

    //This invisible item keeps all the old dismissed tools:
    //note that the outside application still has to keep references to them (or explicitly delete them) or they will just accumulate wasting memory
    Item {
        id: oldToolsItem
        visible: false
    }

    /**
     * This sets the tools for the ToolBar and the transition type that will be
     * used when the page changes on the relevant PageStack.
     *
     * @param type:Item tools see tool property
     * @param type:string transition see transition property
     */
    function setTools(tools, transition)
    {
        if (toolBar.tools == tools) {
            return
        }

        if (connection.oldTools) {
            connection.oldTools.parent = oldToolsItem
        }
        connection.oldTools = toolBar.tools
        toolBar.transition = transition
        toolBar.tools = tools
    }
    Connections {
        id: connection
        target: toolBar
        property Item oldTools

        function internalToolsChanged()
        {
            var newContainer
            var oldContainer
            if (containerA.current) {
                newContainer = containerB
                oldContainer = containerA
            } else {
                newContainer = containerA
                oldContainer = containerB
            }
            containerA.current = !containerA.current

            if (tools) {
                tools.parent = newContainer
                tools.visible = true
                tools.anchors.left = newContainer.left
                tools.anchors.right = newContainer.right
            }

            switch (transition) {
            case "push":
                containerA.animationsEnabled = true
                oldContainer.x = -oldContainer.width/2

                containerA.animationsEnabled = false
                newContainer.x = newContainer.width/2
                newContainer.y = 0
                containerA.animationsEnabled = true
                newContainer.x = 0
                break
            case "pop":
                containerA.animationsEnabled = true
                oldContainer.x = oldContainer.width/2

                containerA.animationsEnabled = false
                newContainer.x = -newContainer.width/2
                newContainer.y = 0
                containerA.animationsEnabled = true
                newContainer.x = 0
                break
            case "replace":
                containerA.animationsEnabled = true
                oldContainer.y = oldContainer.height

                containerA.animationsEnabled = false
                newContainer.x = 0
                newContainer.y = -newContainer.height
                containerA.animationsEnabled = true
                newContainer.y = 0
                break
            case "set":
            default:
                containerA.animationsEnabled = false
                containerA.animationsEnabled = false
                oldContainer.x = -oldContainer.width/2
                newContainer.x = 0
                break
            }

            newContainer.opacity = 1
            oldContainer.opacity = 0
        }
        onToolsChanged: connection.internalToolsChanged()
        Component.onCompleted: connection.internalToolsChanged()
    }

    PlasmaCore.FrameSvgItem {
        id: frameSvg
        imagePath: "widgets/toolbar"
        anchors {
            fill: parent
            leftMargin: -margins.left
            rightMargin: -margins.right
            //FIXME: difference between actial border and shadow
            topMargin: toolBar.y <= 0 ? -margins.top : -margins.top/2
            bottomMargin: toolBar.y >= toolBar.parent.height - toolBar.height ? -margins.bottom : -margins.bottom/2
        }
    }

    Item {
        clip: containerAOpacityAnimation.running || heightAnimation.running
        anchors {
            fill: parent
            leftMargin: frameSvg.margins.left
            topMargin: frameSvg.margins.top
            rightMargin: frameSvg.margins.right
            bottomMargin: frameSvg.margins.bottom
        }

        // We have two containers here so that when we transition from one set of tools to another
        // we can have a nice transition animation with the old tools in one container and the
        // new tools in the other
        Item {
            id: containerA
            width: parent.width
            height: parent.height
            property bool animationsEnabled: false
            opacity: 0
            //this asymmetry just to not export a property
            property bool current: false
            Behavior on opacity {
                PropertyAnimation {
                    id: containerAOpacityAnimation
                    duration: 250
                }
            }
            Behavior on x {
                enabled: containerA.animationsEnabled
                PropertyAnimation {
                    duration: 250
                }
            }
            Behavior on y {
                enabled: containerA.animationsEnabled
                PropertyAnimation {
                    duration: 250
                }
            }
        }
        Item {
            id: containerB
            width: parent.width
            height: parent.height
            opacity: 0
            Behavior on opacity {
                PropertyAnimation { duration: 250 }
            }
            Behavior on x {
                enabled: containerA.animationsEnabled
                PropertyAnimation {
                    duration: 250
                }
            }
            Behavior on y {
                enabled: containerA.animationsEnabled
                PropertyAnimation {
                    duration: 250
                }
            }
        }
    }
}
