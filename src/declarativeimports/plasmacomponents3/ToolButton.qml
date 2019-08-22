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

    flat: true

    contentItem: RowLayout {

        Item {
            Layout.preferredWidth: Kirigami.Units.iconSizes.smallMedium + Kirigami.Units.largeSpacing
            Layout.fillWidth: !label.visible
            visible: icon.source.length > 0
            PlasmaCore.IconItem {
                id: icon
                anchors.centerIn: parent
                height: Kirigami.Units.iconSizes.smallMedium
                width:  Kirigami.Units.iconSizes.smallMedium
                colorGroup: PlasmaCore.ColorScope.colorGroup
                source: control.icon ? (control.icon.name || control.icon.source) : ""
            }
        }

        Label {
            id: label
            Layout.fillWidth: true
            visible: text.length > 0
            text: control.Kirigami.MnemonicData.richTextLabel
            font: control.font
            opacity: enabled || control.highlighted || control.checked ? 1 : 0.4
            color: theme.buttonTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    background: Item {
        visible: (!control.flat || control.hovered) && (!control.pressed || !control.checked)
        //retrocompatibility with old controls
        implicitWidth: units.gridUnit * 1.6
        implicitHeight: units.gridUnit * 1.6
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
