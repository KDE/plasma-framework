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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore

/**
 * Used to highlight an item of a list. to be used only as the "highlight"
 * property of the ListView and GridView primitive QML components (or their
 * derivates)
 *
 * Provides built-in animation of Behavior on opacity Easing.OutQuad for a
 * duration of 250.
 *
 * (TODO, make optional? e.g. animate: false)
 */
Item {
    id: highlight

    /** true if the user is hovering over the component */
    //in the case we are the highlight of a listview, it follows the mouse, so hover = true
    property bool hover: ListView ? true : false

    /** true if the mouse button is pressed over the component. */
    property bool pressed: false
    width: ListView.view ? ListView.view.width : undefined

    Connections {
        target: highlight.ListView.view
        onCurrentIndexChanged: {
            if (highlight.ListView.view.currentIndex >= 0) {
                background.opacity = 1
            } else {
                background.opacity = 0
            }
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: 250
            easing.type: Easing.OutQuad
        }
    }

    PlasmaCore.FrameSvgItem {
        id: background
        imagePath: "widgets/viewitem"
        prefix: {
            if (pressed)
                return hover ? "selected+hover" : "selected";

            if (hover)
                return "hover";

            return "normal";
        }

        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.OutQuad
            }
        }

        anchors {
            fill: parent
        //FIXME: breaks listviews and highlight item
        //    topMargin: -background.margins.top
        //    leftMargin: -background.margins.left
        //    bottomMargin: -background.margins.bottom
        //    rightMargin: -background.margins.right
        }
    }
}
