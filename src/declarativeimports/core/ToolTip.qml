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
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

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

    property int _s: theme.iconSizes.small / 2

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
            width: childrenRect.width + margins.left + margins.right
            height: childrenRect.height + margins.top + margins.bottom

            onWidthChanged: print("XXXX ====================================== svgframe width: " + width)
            onChildrenRectChanged: print("XXXX childrenRect chagned" + childrenRect.width + " " + childrenRect.height)

            Item {
                id: toolTipContentItem
                x: toolTipSvg.margins.left
                y: toolTipSvg.margins.top
                width: childrenRect.width
                height: childrenRect.height
                anchors {
                    topMargin: parent.anchors.margins.top
                    leftMargin: parent.anchors.margins.left
                    //fill: parent
                }

                PlasmaCore.IconItem {
                    id: tooltipIcon
                    width: theme.iconSizes.desktop
                    height: width
                    source: "zanshin"
//                     anchors {
//                         left: parent.left
//                         verticalCenter: parent.verticalCenter
//                     }
                }
                PlasmaExtras.Heading {
                    id: tooltipMaintext
                    level: 4
                    text: "Emperor fish sighted"
                    anchors {
                        left: tooltipIcon.right
                        leftMargin: _s
                        //right: parent.right
                        top: parent.top
                    }
                }
                PlasmaComponents.Label {
                    id: tooltipSubtext
                    text: "You've come across a beautiful sea creature"
                    anchors {
                        left: tooltipIcon.right
                        leftMargin: _s
                        topMargin: _s
                        //right: parent.right
                        top: tooltipMaintext.bottom
                    }
                }

            }

        }

    }
}
