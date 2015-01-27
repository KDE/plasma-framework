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
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons



Row {
    id: tooltipContentItem

    property Item toolTip

    property int preferredTextWidth: theme.mSize(theme.defaultFont).width * 40
    property int _s: units.largeSpacing / 2

    Layout.minimumWidth: implicitWidth + _s
    Layout.minimumHeight: implicitHeight + _s * 2
    Layout.maximumWidth: implicitWidth + _s
    Layout.maximumHeight: implicitHeight + _s * 2
    width: implicitWidth + _s
    height: implicitHeight + _s * 2

    spacing: _s*2

    Item {
        id: imageContainer
        visible: toolTip != null && (toolTip.image != null || toolTip.icon != null)
        width: Math.max(tooltipImage.width, tooltipIcon.width)
        height: Math.max(tooltipImage.height, tooltipIcon.height)
        x: _s
        y: _s

        Image {
            id: tooltipImage
            source: toolTip ? toolTip.image : ""
            x: _s
        }

        PlasmaCore.IconItem {
            id: tooltipIcon
            x: _s
            width: toolTip != undefined && toolTip.icon != null ? units.iconSizes.desktop : 0
            height: width
            source: toolTip != undefined && toolTip.icon != null ? toolTip.icon : ""
        }
    }

    Column {
        id: mainColumn
        x: _s
        y: _s

        PlasmaExtras.Heading {
            id: tooltipMaintext
            level: 3
            width: Math.min(tooltipMaintext.implicitWidth, preferredTextWidth)
            elide: Text.ElideRight
            text: toolTip ? toolTip.mainText : ""
        }
        PlasmaComponents.Label {
            id: tooltipSubtext
            width: Math.min(tooltipSubtext.implicitWidth, preferredTextWidth)
            wrapMode: Text.WordWrap
            text: toolTip ? toolTip.subText : ""
            textFormat: toolTip.textFormat
            opacity: 0.5
        }
    }
}


