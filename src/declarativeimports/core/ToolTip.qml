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

    property string mainText // string
    property string subText // string
    property string iconSource // icon name
    property string image // string / url to the image
    property Item target: parent
    property Component mainComponent: tooltipComponent // custom component to create inside the tooltip

    // private props
    property int _s: theme.iconSizes.small / 2

    hoverEnabled: true

    onEntered: show();
    onExited: hide();

    function show() {
        var mi = tooltip.mainItem;
        if (mi == null) {
            mi = tooltip.mainComponent.createObject( tooltip.target, { "mainText": tooltip.mainText, "subText": tooltip.subText, "iconSource": tooltip.iconSource, "image": tooltip.image });
        }
        tooltipWindow.visualParent = tooltip.target;
        tooltipWindow.mainItem = mi;
        tooltipWindow.visible = true;
        tooltipHideTimer.running = false;
    }

    function hide() {
        tooltipHideTimer.running = true
    }

    Timer {
        id: tooltipHideTimer
        running: false
        repeat: false
        interval: 0
        onTriggered: {
            //print("Hiding tooltip ...");
            tooltipWindow.visible = false;
            tooltipWindow.mainItem.destroy();
        }
    }
    Component {
        id: tooltipComponent

        Item {
            id: tooltipContentItem
            x: _s
            y: _s
            width: childrenRect.width + _s
            height: childrenRect.height

            property string mainText: "" // string
            property string subText: "" // string
            property string iconSource: "" // icon name
            property string image: "" // string / url to the image

            property int maxTextSize: Math.max(tooltipMaintext.paintedWidth, tooltipSubtext.paintedWidth)
            property int maxSize: theme.iconSizes.desktop * 6
            property int preferredTextWidth: Math.min(maxTextSize, maxSize)

            property int _s: theme.iconSizes.small / 2

            Image {
                id: tooltipImage
                source: image
            }

            PlasmaCore.IconItem {
                id: tooltipIcon
                width: theme.iconSizes.desktop
                height: width
                source: iconSource
                anchors {
                    leftMargin: _s
                }
            }
            PlasmaExtras.Heading {
                id: tooltipMaintext
                level: 3
                width: parent.preferredTextWidth
                wrapMode: Text.WordWrap
                text: mainText
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
                text: subText
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
    PlasmaCore.ToolTipProxy {
        id: tooltipWindow
        visualParent: tooltip.target
    }
}
