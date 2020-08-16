/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami
import "private" as Private

T.ToolButton {
    id: control

    implicitWidth: Math.max(units.gridUnit, contentItem.implicitWidth)
                            + leftPadding + rightPadding
    implicitHeight: Math.max(units.gridUnit, contentItem.implicitHeight)
                            + topPadding + bottomPadding

    Layout.minimumWidth: contentItem.implicitWidth + leftPadding + rightPadding

    leftPadding: background.leftMargin
    topPadding: background.topMargin
    rightPadding: background.rightMargin
    bottomPadding: background.bottomMargin

    hoverEnabled: !Kirigami.Settings.tabletMode

    Kirigami.MnemonicData.enabled: control.enabled && control.visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: control.text

    flat: true

    PlasmaCore.ColorScope.inherit: flat
    PlasmaCore.ColorScope.colorGroup: flat ? parent.PlasmaCore.ColorScope.colorGroup : PlasmaCore.Theme.ButtonColorGroup

    contentItem: GridLayout {

        /* Even though keyboardFocus is already defined as a property
         * in the background, it's null when it reaches the contentItem.
         * That means it has to be created here too unless a solution is found.
         */
        property bool keyboardFocus: control.activeFocus &&
        (control.focusReason == Qt.TabFocusReason || control.focusReason == Qt.BacktabFocusReason)

        columns: control.display == T.AbstractButton.TextBesideIcon ? 2 : 1

        PlasmaCore.IconItem {
            id: icon

            Layout.alignment: Qt.AlignCenter

            Layout.fillWidth: true
            Layout.fillHeight: true
            // The default icon size is smallMedium (22px) which means the
            // content item will be too large and undesirably increase the size
            // of the item itself, so we apply negative margins to make it take
            // up as little space as a small icon would
            Layout.margins: -((implicitHeight - units.iconSizes.small) / 2)

            Layout.minimumWidth: Math.min(parent.width, parent.height, implicitWidth)
            Layout.minimumHeight: Math.min(parent.width, parent.height, implicitHeight)

            Layout.maximumWidth: control.icon.width > 0 ? control.icon.width : Number.POSITIVE_INFINITY
            Layout.maximumHeight: control.icon.height > 0 ? control.icon.height : Number.POSITIVE_INFINITY

            implicitWidth: control.icon.width > 0 ? control.icon.width : units.iconSizes.smallMedium
            implicitHeight: control.icon.height > 0 ? control.icon.height : units.iconSizes.smallMedium

            colorGroup: PlasmaCore.ColorScope.colorGroup
            visible: source.length > 0 && control.display !== T.AbstractButton.TextOnly
            source: control.icon ? (control.icon.name || control.icon.source) : ""
            status: !control.flat && contentItem.keyboardFocus && !control.pressed && !control.checked ? PlasmaCore.Svg.Selected : PlasmaCore.Svg.Normal
        }

        Label {
            id: label
            Layout.fillWidth: true
            visible: text.length > 0 && control.display !== T.AbstractButton.IconOnly
            text: control.Kirigami.MnemonicData.richTextLabel
            font: control.font
            opacity: width > 0 ? (enabled || control.highlighted || control.checked ? 1 : 0.4) : 0
            color: (control.hovered || !control.flat) && buttonsurfaceChecker.usedPrefix != "toolbutton-hover" ? theme.buttonTextColor : PlasmaCore.ColorScope.textColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            PlasmaCore.FrameSvgItem {
                id: buttonsurfaceChecker
                visible: false
                imagePath: "widgets/button"
                prefix: control.flat ? ["toolbutton-hover", "normal"] : "normal"
            }
        }
    }

    background: Private.ButtonBackground {
        parentControl: control
    }
}
