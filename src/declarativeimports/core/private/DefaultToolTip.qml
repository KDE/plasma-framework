/*
    SPDX-FileCopyrightText: 2013-2015 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras


PlasmaCore.ColorScope {
    id: tooltipContentItem

    property Item toolTip
    property int preferredTextWidth: units.gridUnit * 20

    Layout.minimumWidth: childrenRect.width + units.gridUnit
    Layout.minimumHeight: childrenRect.height + units.gridUnit
    Layout.maximumWidth: childrenRect.width + units.gridUnit
    Layout.maximumHeight: childrenRect.height + units.gridUnit

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true
    colorGroup: PlasmaCore.Theme.NormalColorGroup
    inherit: false

    RowLayout {

        anchors {
            left: parent.left
            top: parent.top
            margins: units.gridUnit / 2
        }

        spacing: units.largeSpacing

        Image {
            id: tooltipImage
            source: toolTip ? toolTip.image : ""
            visible: toolTip != null && toolTip.image != ""
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }

        PlasmaCore.IconItem {
            id: tooltipIcon
            animated: false
            source: toolTip ? toolTip.icon : ""
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            visible: toolTip != null && toolTip.icon != "" && toolTip.image == "" && valid
            Layout.preferredWidth: units.iconSizes.medium
            Layout.preferredHeight: units.iconSizes.medium
        }

        ColumnLayout {
            Layout.maximumWidth: preferredTextWidth
            spacing: 0

            PlasmaExtras.Heading {
                id: tooltipMaintext
                level: 3
                Layout.fillWidth: true
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                text: toolTip ? toolTip.mainText : ""
                textFormat: Text.PlainText
                visible: text != ""
            }
            PlasmaComponents.Label {
                id: tooltipSubtext
                Layout.fillWidth: true
                // Unset Label default height, confuses the layout engine completely
                // either shifting the item vertically or letting it get too wide
                height: undefined
                wrapMode: Text.WordWrap
                text: toolTip ? toolTip.subText : ""
                textFormat: toolTip ? toolTip.textFormat : Text.AutoText
                opacity: 0.6
                visible: text != ""
                maximumLineCount: 8
            }
        }
    }
}

