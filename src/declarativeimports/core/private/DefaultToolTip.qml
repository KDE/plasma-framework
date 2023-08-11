/*
    SPDX-FileCopyrightText: 2013-2015 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2 as Kirigami

Item {
    property Item toolTip
    property int preferredTextWidth: Kirigami.Units.gridUnit * 20

    implicitWidth: mainLayout.implicitWidth + Kirigami.Units.largeSpacing * 2
    implicitHeight: mainLayout.implicitHeight + Kirigami.Units.largeSpacing * 2

    LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
    LayoutMirroring.childrenInherit: true

    Kirigami.Theme.colorSet: Kirigami.Theme.Window
    Kirigami.Theme.inherit: false

    RowLayout {
        id: mainLayout
        anchors.fill: parent
        anchors.margins: Kirigami.Units.largeSpacing

        spacing: Kirigami.Units.gridUnit

        Image {
            source: toolTip ? toolTip.image : ""
            visible: toolTip !== null && toolTip.image !== ""
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
        }

        Kirigami.Icon {
            animated: false
            source: toolTip ? toolTip.icon : ""
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            visible: toolTip !== null && toolTip.icon !== "" && toolTip.image === "" && valid
            Layout.preferredWidth: Kirigami.Units.iconSizes.medium
            Layout.preferredHeight: Kirigami.Units.iconSizes.medium
        }

        ColumnLayout {
            Layout.maximumWidth: preferredTextWidth
            spacing: 0

            Kirigami.Heading {
                level: 3
                Layout.fillWidth: true
                elide: Text.ElideRight
                wrapMode: Text.Wrap
                text: toolTip ? toolTip.mainText : ""
                textFormat: Text.PlainText
                visible: text !== ""
            }

            PlasmaComponents.Label {
                Layout.fillWidth: true
                wrapMode: Text.WordWrap
                text: toolTip ? toolTip.subText : ""
                textFormat: toolTip ? toolTip.textFormat : Text.AutoText
                opacity: 0.6
                visible: text !== ""
                maximumLineCount: 8
            }
        }
    }
}
