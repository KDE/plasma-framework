/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQml 2.15
import QtQuick.Window 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.Control {
    id: root
    property int alignment: 0 // Null alignment
    property int display: T.AbstractButton.TextBesideIcon
    property alias labelText: label.text
    property alias labelColor: label.color
    property alias labelLinkColor: label.linkColor
    property alias labelElide: label.elide
    property alias iconWidth: iconItem.implicitWidth
    property alias iconHeight: iconItem.implicitHeight
    property alias iconSource: iconItem.source
    property alias iconOverlays: iconItem.overlays
    property alias iconActive: iconItem.active
    property alias iconAnimated: iconItem.animated
    property alias iconUsesPlasmaTheme: iconItem.usesPlasmaTheme
    property alias iconRoundToIconSize: iconItem.roundToIconSize
    property alias iconValid: iconItem.valid
    PlasmaCore.ColorScope.inherit: true
    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    contentItem: GridLayout {
        rowSpacing: root.spacing
        columnSpacing: root.spacing
        flow: root.display === T.AbstractButton.TextUnderIcon ? GridLayout.TopToBottom : GridLayout.LeftToRight
        PlasmaCore.IconItem {
            id: iconItem
            visible: valid && width > 0 && height > 0 && root.display !== T.AbstractButton.TextOnly
            colorGroup: root.PlasmaCore.ColorScope.colorGroup
            implicitWidth: PlasmaCore.Units.iconSizes.sizeForLabels
            implicitHeight: PlasmaCore.Units.iconSizes.sizeForLabels
            Layout.alignment: if (root.display === T.AbstractButton.TextBesideIcon && label.visible) {
                Qt.AlignRight | Qt.AlignVCenter
            } else if (root.display === T.AbstractButton.TextUnderIcon && label.visible) {
                Qt.AlignHCenter | Qt.AlignBottom
            } else {
                Qt.AlignCenter
            }
        }
        T.Label {
            id: label
            visible: text.length > 0 && root.display !== T.AbstractButton.IconOnly
            palette: root.palette
            font: root.font
            horizontalAlignment: root.alignment & Qt.AlignHorizontal_Mask || Text.AlignHCenter
            verticalAlignment: root.alignment & Qt.AlignVertical_Mask || Text.AlignVCenter
            // Work around Qt bug where NativeRendering breaks for non-integer scale factors
            // https://bugreports.qt.io/browse/QTBUG-70481
            renderType: Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering
            color: PlasmaCore.ColorScope.textColor
            linkColor: PlasmaCore.Theme.linkColor
            elide: Text.ElideRight
            Layout.fillWidth: root.alignment & Qt.AlignHorizontal_Mask
            Layout.fillHeight: root.alignment & Qt.AlignVertical_Mask
            Layout.alignment: if (root.display === T.AbstractButton.TextBesideIcon && iconItem.visible) {
                Qt.AlignLeft | Qt.AlignVCenter
            } else if (root.display === T.AbstractButton.TextUnderIcon && iconItem.visible) {
                Qt.AlignHCenter | Qt.AlignTop
            } else {
                Qt.AlignCenter
            }
        }
    }
}
