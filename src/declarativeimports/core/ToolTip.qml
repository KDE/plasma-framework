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
    id: tooltip

    property alias mainText: tooltipMaintext.text // string
    property alias subText: tooltipSubtext.text // string
    property alias iconSource: tooltipIcon.source // icon name
    property alias image: tooltipImage.source // string / url to the image
    property Item target: parent

    // private props
    property int _s: theme.iconSizes.small / 2

    hoverEnabled: true
    onEntered: {
        print("entered");
        tooltipWindow.visible = true;
        tooltipHideTimer.running = false;
    }
    onExited: {
        print("exit");
        tooltipHideTimer.running = true
    }

    Timer {
        id: tooltipHideTimer
        running: false
        repeat: false
        interval: 50
        onTriggered: {
            //print("Hiding tooltip ...");
            tooltipWindow.visible = false;
        }
    }

    PlasmaCore.ToolTipProxy {
        id: tooltipWindow
        visualParent: tooltip.target

        mainItem: PlasmaCore.FrameSvgItem {
            id: tooltipSvg
            //imagePath: "widgets/tooltip"
            width: childrenRect.width + margins.left + margins.right + 2*_s
            height: childrenRect.height + margins.top + margins.bottom + 2*_s

            Item {
                id: tooltipContentItem
                x: tooltipSvg.margins.left + _s
                y: tooltipSvg.margins.top + _s
                width: childrenRect.width + _s
                height: childrenRect.height

                property int maxTextSize: Math.max(tooltipMaintext.paintedWidth, tooltipSubtext.paintedWidth)
                property int maxSize: theme.iconSizes.desktop * 6
                property int preferredTextWidth: Math.min(maxTextSize, maxSize)

                Image {
                    id: tooltipImage
                }

                PlasmaCore.IconItem {
                    id: tooltipIcon
                    width: theme.iconSizes.desktop
                    height: width
                    anchors {
                        leftMargin: _s
                    }
                }
                PlasmaExtras.Heading {
                    id: tooltipMaintext
                    level: 3
                    width: parent.preferredTextWidth
                    wrapMode: Text.WordWrap
                    text: tooltipWindow.mainText
                    anchors {
                        left: (tooltipImage.source != "") ? tooltipImage.right : tooltipIcon.right
                        leftMargin: _s*2
                        top: tooltipIcon.top
                    }
                }
                PlasmaComponents.Label {
                    id: tooltipSubtext
                    width: parent.preferredTextWidth
                    wrapMode: Text.WordWrap
                    text: tooltipWindow.subText
                    opacity: 0.5
                    anchors {
                        left: tooltipMaintext.left
                        topMargin: _s
                        bottomMargin: _s
                        top: tooltipMaintext.bottom
                    }
                }
            }
        }
    }
}
