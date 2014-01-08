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
import org.kde.qtextracomponents 2.0 as QtExtras

/**
 * An Item managing a Plasma-themed tooltip. It is rendered in its own window.
 * You can either specify iconSource, mainText and subText, or a custom Component
 * that will be put inside the tooltip. By specifying the target property, you
 * "attach" the ToolTip to an item in your code, by default the tooltip will be
 * rendered when hovering over the parent item.
 *
 * The item inside the ToolTip is loaded on demand and will be destroyed when the
 * tooltip is being hidden.
 *
 * Example usage:
 * @code
 * import org.kde.plasma.core 2.0 as PlasmaCore
 *
 * [...]
 * PlasmaComponents.IconItem {
 *     ...
 *     PlasmaCore.ToolTip {
 *         mainText: "Tooltip Title"
 *         subText: "Some explanation."
 *         iconSource: "plasma"
 *         // alternatively, you can specify your own component
 *         // to be loaded when the tooltip shows
 *         mainComponent: Component {
 *              YourCustomItem { ...  }
 *         }
 * ... }
 * }
 * @endcode
 *
 */

Item {
    id: tooltipContentItem
    x: _s
    y: _s
    width: childrenRect.width + _s
    height: childrenRect.height
    property Item toolTip

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
        source: toolTip ? toolTip.image : ""
    }

    PlasmaCore.IconItem {
        id: tooltipIcon
        width: toolTip.icon != "" ? theme.iconSizes.desktop : 0
        height: width
        source: toolTip ? toolTip.icon : ""
        anchors {
            leftMargin: width != 0 ? _s : 0
        }
    }
    PlasmaExtras.Heading {
        id: tooltipMaintext
        level: 3
        wrapMode: Text.WordWrap
        text: toolTip ? toolTip.mainText : ""
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
        text: toolTip ? toolTip.subText : ""
        opacity: 0.5
        anchors {
            left: tooltipMaintext.left
            topMargin: _s
            bottomMargin: _s
            top: tooltipMaintext.bottom
        }
    }
}


