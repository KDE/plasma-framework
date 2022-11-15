/*
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.1 as PlasmaCore

Item {
    id: root

    // alignment is subject to mirroring
    property int alignment: 0 // Null alignment
    property int effectiveHorizontalAlignment: !LayoutMirroring.enabled ? alignment
        : (alignment & Qt.AlignLeft) ? Qt.AlignRight
        : (alignment & Qt.AlignRight) ? Qt.AlignLeft
        : Qt.AlignHCenter
    property int display: T.AbstractButton.TextBesideIcon
    readonly property bool iconOnly: display === T.AbstractButton.IconOnly || !label.visible
    readonly property bool textOnly: display === T.AbstractButton.TextOnly || !iconItem.visible
    readonly property bool textBesideIcon: display === T.AbstractButton.TextBesideIcon && iconItem.visible && label.visible
    readonly property bool textUnderIcon: display === T.AbstractButton.TextUnderIcon && iconItem.visible && label.visible

    property alias iconItem: iconItem
    property alias label: label

    property real topPadding: 0
    property real leftPadding: 0
    property real rightPadding: 0
    property real bottomPadding: 0

    readonly property real availableWidth: width - leftPadding - rightPadding
    readonly property real availableHeight: height - topPadding - bottomPadding

    property real spacing: 0

    property font font
    // TODO KF6: remove palette property, as it is already built-in into all Items in Qt 6
    property var palette
    // TODO KF6: This is not a correct formula for mirrored property.
    // Explicitly setting `LayoutMirroring.enabled` to `false` should undone
    // any mirroring imposed by LayoutMirroring inheritance or RTL locale.
    // Fixed in Qt 6.2, see QTBUG-91227
    property bool mirrored: false

    PlasmaCore.ColorScope.inherit: true

    implicitWidth: gridLayout.implicitWidth + leftPadding + rightPadding
    implicitHeight: gridLayout.implicitHeight + topPadding + bottomPadding

    GridLayout {
        id: gridLayout

        rowSpacing: root.spacing
        columnSpacing: root.spacing
        flow: root.display === T.AbstractButton.TextUnderIcon ? GridLayout.TopToBottom : GridLayout.LeftToRight
        // Avoid manipulating layoutDirection directly, and we still need to
        // set LayoutMirroring.enabled to some deterministic value which
        // would not be affected by a random LayoutMirroring.childrenInherit
        // up the parents chain.
        LayoutMirroring.enabled: root.mirrored
        x: {
            if (root.effectiveHorizontalAlignment & Qt.AlignLeft) {
                return root.leftPadding;
            }
            if (root.effectiveHorizontalAlignment & Qt.AlignRight) {
                return root.width - width - root.rightPadding;
            }
            return Math.round((root.availableWidth - width) / 2);
        }
        y: {
            if (root.alignment & Qt.AlignTop) {
                return root.topPadding;
            }
            if (root.alignment & Qt.AlignBottom) {
                return root.height - height - root.bottomPadding;
            }
            return Math.round((root.availableHeight - height) / 2);
        }
        width: Math.min(root.availableWidth, implicitWidth)
        height: Math.min(root.availableHeight, implicitHeight)

        PlasmaCore.IconItem {
            id: iconItem
            visible: valid && width > 0 && height > 0 && root.display !== T.AbstractButton.TextOnly
            colorGroup: root.PlasmaCore.ColorScope.colorGroup
            implicitWidth: PlasmaCore.Units.iconSizes.sizeForLabels
            implicitHeight: PlasmaCore.Units.iconSizes.sizeForLabels
            Layout.alignment: Qt.AlignCenter
            Layout.maximumWidth: implicitWidth > 0 ? implicitWidth : Number.POSITIVE_INFINITY
            Layout.maximumHeight: implicitHeight > 0 ? implicitHeight : Number.POSITIVE_INFINITY
        }

        T.Label {
            id: label
            visible: text.length > 0 && root.display !== T.AbstractButton.IconOnly
            palette: root.palette
            font: root.font
            color: PlasmaCore.ColorScope.textColor
            linkColor: PlasmaCore.Theme.linkColor
            elide: Text.ElideRight
            Layout.alignment: Qt.AlignCenter
            Layout.fillWidth: true
        }
    }
}
