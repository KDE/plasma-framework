/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2016 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.ksvg 1.0 as KSvg
//NOTE: importing PlasmaCore is necessary in order to make KSvg load the current Plasma Theme
import org.kde.plasma.core 2 as PlasmaCore
import org.kde.kirigami 2 as Kirigami

T.ToolTip {
    id: control

    x: parent ? Math.round((parent.width - implicitWidth) / 2) : 0
    y: -implicitHeight - 3

    visible: parent instanceof T.AbstractButton && (Kirigami.Settings.tabletMode ? parent.pressed : parent.hovered) && text.length > 0
    delay: Kirigami.Settings.tabletMode ? Qt.styleHints.mousePressAndHoldInterval : Kirigami.Units.toolTipDelay
    // Never time out while being hovered; it's annoying
    timeout: -1

    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset, contentHeight + topPadding + bottomPadding)
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, contentWidth + leftPadding + rightPadding)

    margins: Kirigami.Units.smallSpacing
    leftPadding: backgroundItem.margins.left
    topPadding: backgroundItem.margins.top
    rightPadding: backgroundItem.margins.right
    bottomPadding: backgroundItem.margins.bottom

    enter: Transition {
        NumberAnimation {
            property: "opacity"
            from: 0.0
            to: 1.0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }

    exit: Transition {
        NumberAnimation {
            property: "opacity"
            from: 1.0
            to: 0.0
            duration: Kirigami.Units.longDuration
            easing.type: Easing.OutCubic
        }
    }

    closePolicy: T.Popup.CloseOnEscape | T.Popup.CloseOnPressOutsideParent | T.Popup.CloseOnReleaseOutsideParent

    contentItem: Item {
        implicitWidth: Math.min(label.maxTextLength, label.contentWidth)
        implicitHeight: label.implicitHeight

        Label {
            id: label

            // This value is basically arbitrary. It just looks nice.
            readonly property double maxTextLength: Kirigami.Units.gridUnit * 14

            // Strip out ampersands right before non-whitespace characters, i.e.
            // those used to determine the alt key shortcut
            text: control.text.replace(/&(?=\S)/g, "")
            wrapMode: Text.WordWrap
            font: control.font

            Kirigami.Theme.colorGroup: Kirigami.Theme.Tooltip
            Kirigami.Theme.inherit: false

            // ensure that long text actually gets wrapped
            onLineLaidOut: (line) => {
                if (line.implicitWidth > maxTextLength)
                    line.width = maxTextLength
            }

        }
    }

    background: Item {
        implicitHeight: Kirigami.Units.gridUnit + backgroundItem.margins.top + backgroundItem.margins.bottom
        implicitWidth: Kirigami.Units.gridUnit + backgroundItem.margins.left + backgroundItem.margins.right

        KSvg.FrameSvgItem {
            anchors.fill: parent
            anchors.leftMargin: -margins.left
            anchors.rightMargin: -margins.right
            anchors.topMargin: -margins.top
            anchors.bottomMargin: -margins.bottom
            imagePath: "solid/widgets/tooltip"
            prefix: "shadow"
            Kirigami.Theme.colorGroup: Kirigami.Theme.Tooltip
            Kirigami.Theme.inherit: false
        }

        KSvg.FrameSvgItem {
            id: backgroundItem
            anchors.fill: parent
            // Because the transparent one doesn't match the appearance of all
            // other ones
            imagePath: "solid/widgets/tooltip"
            Kirigami.Theme.colorGroup: Kirigami.Theme.Tooltip
            Kirigami.Theme.inherit: false
        }
    }
}
