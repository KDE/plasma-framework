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

    property alias mainText: tooltipWindow.mainText
    property alias subText: tooltipWindow.subText
    property alias image: tooltipWindow.image


    property int _s: theme.iconSizes.small / 2

    hoverEnabled: true
    onEntered: {
        print("entere");
        tooltipWindow.visible = true;
        tooltipHideTimer.running = false;
        print(" ttsvg: w " + tooltipSvg.width);
        print(" ttsvg: h " + tooltipSvg.height);

    }

    onExited: {
        print("exit");
        tooltipHideTimer.running = true

    }


    Timer {
        id: tooltipHideTimer
        running: false
        repeat: false
        interval: 100


        onTriggered: {
            print("Hiding tooltip ...");
            tooltipWindow.visible = false;
        }
    }

    PlasmaCore.ToolTipWindow {
        id: tooltipWindow

        mainItem: PlasmaCore.FrameSvgItem {
            id: tooltipSvg
            imagePath: "widgets/tooltip"
            width: childrenRect.width + margins.left + margins.right + 2*_s
            height: childrenRect.height + margins.top + margins.bottom + 2*_s

            onWidthChanged: print("XXXX ====================================== svgframe width: " + width)
            onChildrenRectChanged: print("XXXX childrenRect chagned" + childrenRect.width + " " + childrenRect.height)

            Item {
                id: tooltipContentItem
                x: tooltipSvg.margins.left
                y: tooltipSvg.margins.top
                width: childrenRect.width
                height: childrenRect.height
                anchors {
                    topMargin: parent.anchors.margins.top
                    leftMargin: parent.anchors.margins.left + _s
                    rightMargin: _s
                    //fill: parent
                }

                // FIXME: Image { source: tooltipWindow.image }

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
                    text: tooltipWindow.mainText
                    anchors {
                        left: tooltipIcon.right
                        leftMargin: _s
                        topMargin: _s
                        top: parent.top
                    }
                }
                PlasmaComponents.Label {
                    id: tooltipSubtext
                    text: tooltipWindow.subText
                    anchors {
                        left: tooltipIcon.right
                        leftMargin: _s
                        topMargin: _s
                        bottomMargin: _s
                        top: tooltipMaintext.bottom
                    }
                }

            }

        }

    }
}
