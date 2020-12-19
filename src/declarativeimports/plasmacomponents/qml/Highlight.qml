/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * Used to highlight an item of a list. to be used only as the "highlight"
 * property of the ListView and GridView primitive QML components (or their
 * derivates)
 *
 * Provides built-in animation of Behavior on opacity Easing.OutQuad for a
 * duration of 50ms (defined in PlasmaCore.Units.veryShortDuration).
 *
 * (TODO, make optional? e.g. animate: false)
 *
 * @inherit QtQuick.Item
 */
Item {
    id: highlight

    /** true if the user is hovering over the component */
    //in the case we are the highlight of a listview, it follows the mouse, so hover = true
    property bool hover: ListView ? true : false

    /** true if the mouse button is pressed over the component. */
    property bool pressed: false
    width: ListView.view ? ListView.view.width : undefined
    property alias marginHints: background.margins;

    Connections {
        target: highlight.ListView.view
        function onCurrentIndexChanged() {
            if (highlight.ListView.view.currentIndex >= 0) {
                background.opacity = 1
            } else {
                background.opacity = 0
            }
        }
    }

    Behavior on opacity {
        NumberAnimation {
            duration: PlasmaCore.Units.veryShortDuration
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
                duration: PlasmaCore.Units.veryShortDuration
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
