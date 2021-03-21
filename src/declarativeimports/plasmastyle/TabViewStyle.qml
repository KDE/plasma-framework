/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Private 1.0
import QtQuick.Controls.Styles 1.2 as QtQuickControlStyle
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents


/**
 * \internal
 */
QtQuickControlStyle.TabViewStyle {
    tabsMovable: false

    tabsAlignment: Qt.AlignLeft

    tabOverlap: 1

    frameOverlap: 2

    /*! This defines the tab frame. */
    property Component frame: Item {}


    property Component tab: Item {
        property int totalOverlap: tabOverlap * (control.count - 1)
        property real maxTabWidth: control.count > 0 ? (styleData.availableWidth + totalOverlap) / control.count : 0

        implicitWidth: Math.round(Math.min(maxTabWidth, textitem.implicitWidth + 20))
        implicitHeight: Math.round(textitem.implicitHeight + 10)

        PlasmaCore.FrameSvgItem {
            anchors.fill: parent
            opacity: styleData.selected ? 1 : (styleData.hovered ? 0.4 : 0)
            imagePath: "widgets/tabbar"
            prefix: control.tabPosition === Qt.TopEdge ? "north-active-tab" : "south-active-tab"
            colorGroup: PlasmaCore.ColorScope.colorGroup
            Behavior on opacity {
                PropertyAnimation {
                    duration: PlasmaCore.Units.longDuration
                }
            }
        }

        PlasmaComponents.Label {
            id: textitem
            anchors.fill: parent
            anchors.leftMargin: 4
            anchors.rightMargin: 4
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            text: styleData.title
            elide: Text.ElideMiddle
        }
    }

    property Component leftCorner: null

    property Component rightCorner: null

    property Component tabBar: null
}
