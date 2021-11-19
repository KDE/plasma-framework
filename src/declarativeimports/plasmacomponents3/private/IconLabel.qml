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
    readonly property bool iconOnly: display === T.AbstractButton.IconOnly || !label.visible
    readonly property bool textOnly: display === T.AbstractButton.TextOnly || !iconItem.visible
    readonly property bool textBesideIcon: display === T.AbstractButton.TextBesideIcon && iconItem.visible && label.visible
    readonly property bool textUnderIcon: display === T.AbstractButton.TextUnderIcon && iconItem.visible && label.visible

    property alias iconItem: iconItem
    property alias label: label

    PlasmaCore.ColorScope.inherit: true
    implicitWidth: implicitContentWidth + leftPadding + rightPadding
    implicitHeight: implicitContentHeight + topPadding + bottomPadding
    contentItem: Item {
        implicitWidth: gridLayout.implicitWidth
        implicitHeight: gridLayout.implicitHeight
        GridLayout {
            id: gridLayout
            rowSpacing: root.spacing
            columnSpacing: root.spacing
            flow: root.display === T.AbstractButton.TextUnderIcon ? GridLayout.TopToBottom : GridLayout.LeftToRight
            x: {
                if (root.alignment & Qt.AlignLeft) {
                    return 0;
                }
                if (root.alignment & Qt.AlignRight) {
                    return parent.width - width;
                }
                return Math.round((parent.width - width) / 2);
            }
            y: {
                if (root.alignment & Qt.AlignTop) {
                    return 0;
                }
                if (root.alignment & Qt.AlignBottom) {
                    return parent.height - height;
                }
                return Math.round((parent.height - height) / 2);
            }
            width: Math.min(parent.width, implicitWidth)
            height: Math.min(parent.height, implicitHeight)
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
                // Work around Qt bug where NativeRendering breaks for non-integer scale factors
                // https://bugreports.qt.io/browse/QTBUG-70481
                renderType: Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering
                color: PlasmaCore.ColorScope.textColor
                linkColor: PlasmaCore.Theme.linkColor
                elide: Text.ElideRight
                Layout.alignment: Qt.AlignCenter
                Layout.fillWidth: {
                    if (!iconItem.visible || parent.flow === GridLayout.TopToBottom) {
                        return implicitWidth > parent.width;
                    }
                    return iconItem.implicitWidth + parent.columnSpacing + implicitWidth > parent.width;
                }
            }
        }
    }
}
