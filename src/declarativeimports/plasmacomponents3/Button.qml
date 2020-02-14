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

T.Button {
    id: control

    implicitWidth: Math.max(background.implicitWidth,
                            contentItem.implicitWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background.implicitHeight, contentItem.implicitHeight + topPadding + bottomPadding)

    leftPadding: surfaceNormal.margins.left
    topPadding: surfaceNormal.margins.top
    rightPadding: surfaceNormal.margins.right
    bottomPadding: surfaceNormal.margins.bottom

    hoverEnabled: !Kirigami.Settings.tabletMode

    Kirigami.MnemonicData.enabled: control.enabled && control.visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: control.text

    contentItem: RowLayout {
        PlasmaCore.IconItem {
            id: icon

            Layout.alignment: Qt.AlignCenter
            Layout.preferredWidth: control.icon.width > 0 ? control.icon.width : units.iconSizes.small
            Layout.preferredHeight: control.icon.height > 0 ? control.icon.height : units.iconSizes.small

            colorGroup: PlasmaCore.Theme.ButtonColorGroup
            visible: source.length > 0
            source: control.icon ? (control.icon.name || control.icon.source) : ""
            status: buttonSvg.hasElement("hint-focus-highlighted-background") && control.activeFocus && !control.pressed && !control.checked ? PlasmaCore.Svg.Selected : PlasmaCore.Svg.Normal
        }
        Label {
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: text.length > 0
            text: control.Kirigami.MnemonicData.richTextLabel
            font: control.font
            opacity: enabled || control.highlighted || control.checked ? 1 : 0.4
            color: buttonSvg.hasElement("hint-focus-highlighted-background") && control.activeFocus && !control.down ? theme.highlightedTextColor : theme.buttonTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    background: Item {
        //retrocompatibility with old controls
        implicitWidth: Math.floor(units.gridUnit * 1.6) + Math.floor(units.gridUnit * 1.6) % 2
        implicitHeight: implicitWidth
        Private.ButtonShadow {
            anchors.fill: parent
            visible: (!control.flat || control.hovered) && (!control.pressed || !control.checked)
            state: {
                if (control.pressed) {
                    return "hidden"
                } else if (control.hovered) {
                    return "hover"
                } else if (control.activeFocus) {
                    return "focus"
                } else {
                    return "shadow"
                }
            }
        }
        PlasmaCore.Svg {
            id: buttonSvg
            imagePath: "widgets/button"
        }
        PlasmaCore.FrameSvgItem {
            id: surfaceNormal
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: control.activeFocus ? ["focus-background", "normal"] : "normal"
            opacity: (!control.flat || control.hovered) && (!control.pressed || !control.checked) ? 1 : 0
            Behavior on opacity {
                OpacityAnimator {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
        PlasmaCore.FrameSvgItem {
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "pressed"
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
