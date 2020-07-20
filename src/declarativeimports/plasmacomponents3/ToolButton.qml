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

    // for flat toolbuttons, we want to use margins from the pressed state since
    // those are the ones which are appropriate for a button without a buttonlike
    // appearance where we increase the size of the icon compared to buttonlike
    // buttons
    leftPadding: control.flat ? surfacePressed.margins.left : surfaceNormal.margins.left
    topPadding: control.flat ? surfacePressed.margins.top : surfaceNormal.margins.top
    rightPadding: control.flat ? surfacePressed.margins.right : surfaceNormal.margins.right
    bottomPadding: control.flat ? surfacePressed.margins.bottom : surfaceNormal.margins.bottom

    hoverEnabled: !Kirigami.Settings.tabletMode

    Kirigami.MnemonicData.enabled: control.enabled && control.visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: control.text

    flat: true

    PlasmaCore.ColorScope.inherit: flat
    PlasmaCore.ColorScope.colorGroup: flat ? parent.PlasmaCore.ColorScope.colorGroup : PlasmaCore.Theme.ButtonColorGroup

    contentItem: GridLayout {
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

            colorGroup: control.PlasmaCore.ColorScope.colorGroup
            visible: source.length > 0 && control.display !== T.AbstractButton.TextOnly
            source: control.icon ? (control.icon.name || control.icon.source) : ""
            status: !control.flat && control.activeFocus && !control.pressed && !control.checked ? PlasmaCore.Svg.Selected : PlasmaCore.Svg.Normal
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

    background: Item {
        Private.ButtonShadow {
            anchors.fill: parent
            readonly property bool keyboardFocus: control.activeFocus &&
                (control.focusReason == Qt.TabFocusReason || control.focusReason == Qt.BacktabFocusReason)
            visible:  (!control.flat || control.hovered || keyboardFocus) && (!control.pressed || !control.checked)

            state: {
                if (control.pressed) {
                    return "hidden"
                } else if (control.hovered) {
                    return "hover"
                } else if (keyboardFocus) {
                    return "focus"
                } else {
                    return control.flat ? "shadow" : "hidden"
                }
            }
        }
        PlasmaCore.FrameSvgItem {
            id: surfaceNormal
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "normal"
            opacity: !control.flat && (!control.pressed || !control.checked) ? 1 : 0
            Behavior on opacity {
                OpacityAnimator {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
        PlasmaCore.FrameSvgItem {
            id: surfacePressed
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: control.flat ? "toolbutton-pressed" : "pressed"
            opacity: control.checked || control.pressed ? 1 : 0
            Behavior on opacity {
                OpacityAnimator {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
