/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Controls @QQC2_VERSION@
import QtQml.Models 2.1
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami

T.TabButton {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            implicitIndicatorWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)
    baselineOffset: contentItem.y + contentItem.baselineOffset

    padding: PlasmaCore.Units.smallSpacing
    spacing: PlasmaCore.Units.smallSpacing

    hoverEnabled: true

    Kirigami.MnemonicData.enabled: control.enabled && control.visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: control.text

    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(control.text))
        sequence: control.Kirigami.MnemonicData.sequence
        onActivated: control.clicked()
    }

    icon.width: PlasmaCore.Units.iconSizes.smallMedium
    icon.height: PlasmaCore.Units.iconSizes.smallMedium

    contentItem: GridLayout {
        columns: control.display == T.AbstractButton.TextBesideIcon ? 2 : 1
        rowSpacing: control.spacing
        columnSpacing: control.spacing
        PlasmaCore.IconItem {
            id: icon

            Layout.alignment: control.display != T.AbstractButton.TextBesideIcon || !label.visible ? Qt.AlignCenter : Qt.AlignVCenter | Qt.AlignRight

            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.minimumWidth: Math.min(parent.width, parent.height, implicitWidth)
            Layout.minimumHeight: Math.min(parent.width, parent.height, implicitHeight)

            Layout.maximumWidth: control.icon.width > 0 ? control.icon.width : Number.POSITIVE_INFINITY
            Layout.maximumHeight: control.icon.height > 0 ? control.icon.height : Number.POSITIVE_INFINITY

            implicitWidth: control.icon.width
            implicitHeight: control.icon.height

            colorGroup: control.PlasmaCore.ColorScope.colorGroup
            visible: source.length > 0 && control.display !== T.AbstractButton.TextOnly
            source: control.icon ? (control.icon.name || control.icon.source) : ""
            active: control.visualFocus
        }
        Label {
            id: label
            Layout.fillWidth: true
            // allow it to shrink below implicitWidth, but not exceed implicitWidth
            Layout.maximumWidth: Math.ceil(implicitWidth)
            Layout.alignment: horizontalAlignment | verticalAlignment
            visible: text.length > 0 && control.display !== T.AbstractButton.IconOnly
            text: control.Kirigami.MnemonicData.richTextLabel
            font: control.font
            color: control.visualFocus ? PlasmaCore.ColorScope.highlightColor : PlasmaCore.ColorScope.textColor
            opacity: enabled || control.highlighted || control.checked ? 1 : 0.4
            horizontalAlignment: control.display != T.AbstractButton.TextBesideIcon || !icon.visible ? Text.AlignHCenter : Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }
}
