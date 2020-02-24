/*
 *   Copyright 2016 Marco Martin <mart@kde.org>
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding
    baselineOffset: contentItem.y + contentItem.baselineOffset

    padding: units.smallSpacing

    hoverEnabled: true

    Kirigami.MnemonicData.enabled: control.enabled && control.visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: control.text

    contentItem: GridLayout {
        columns: control.display == T.AbstractButton.TextBesideIcon ? 2 : 1
        PlasmaCore.IconItem {
            id: icon

            Layout.alignment: Qt.AlignCenter

            Layout.fillWidth: true
            Layout.fillHeight: true

            Layout.minimumWidth: Math.min(parent.width, parent.height, implicitWidth)
            Layout.minimumHeight: Math.min(parent.width, parent.height, implicitHeight)

            Layout.maximumWidth: control.icon.width > 0 ? control.icon.width : Number.POSITIVE_INFINITY
            Layout.maximumHeight: control.icon.height > 0 ? control.icon.height : Number.POSITIVE_INFINITY

            implicitWidth: control.icon.width > 0 ? control.icon.width : units.iconSizes.smallMedium
            implicitHeight: control.icon.height > 0 ? control.icon.height : units.iconSizes.smallMedium

            colorGroup: control.PlasmaCore.ColorScope.colorGroup
            visible: source.length > 0 && control.display !== T.AbstractButton.TextOnly
            source: control.icon ? (control.icon.name || control.icon.source) : ""
        }
        Label {
            id: label
            Layout.fillWidth: true
            visible: text.length > 0 && control.display !== T.AbstractButton.IconOnly
            text: control.Kirigami.MnemonicData.richTextLabel
            font: control.font
            opacity: enabled || control.highlighted || control.checked ? 1 : 0.4
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    background: Item {}
}
