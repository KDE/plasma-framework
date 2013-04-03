/*
*   Copyright 2013 by Sebastian Kügler <sebas@kde.org>
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

/**
 * This is a Plasma-themed tooltip. It is rendered in its own window
 *
 * Example usage:
 * @code
 * import org.kde.plasma.core 2.0 as PlasmaCore
 * import org.kde.plasma.components 2.0 as PlasmaComponents
 * [...]
 * PlasmaComponents.IconItem {
 *     PlasmaCore.ToolTip { ... }
 *   [...]
 * }
 * @endcode
 *
 */
MouseArea {

    hoverEnabled: true
    onEntered: {
        print("entere");
        toolTipWindow.visible = true;
        toolTipHideTimer.running = false;
        print(" ttsvg: w " + toolTipSvg.width);
        print(" ttsvg: h " + toolTipSvg.height);

    }

    onExited: {
        print("exit");
        toolTipHideTimer.running = true

    }


    Timer {
        id: toolTipHideTimer
        running: false
        repeat: false
        interval: 100


        onTriggered: {
            print("Hiding tooltip ...");
            toolTipWindow.visible = false;
        }
    }

    PlasmaCore.ToolTipWindow {
        id: toolTipWindow

        mainItem: PlasmaCore.FrameSvgItem {
            id: toolTipSvg
            imagePath: "widgets/tooltip"
            width: childrenRect.width + 50
            height: childrenRect.height + 50

            Item {
                width: 300
                height: 200
                anchors {
                    topMargin: parent.anchors.margins.top
                    leftMargin: parent.anchors.margins.left
                    fill: parent
                }
                id: toolTipContentItem
                Rectangle { width: 280; height: 140; color: "orange"; }

            }

        }

    }
}
