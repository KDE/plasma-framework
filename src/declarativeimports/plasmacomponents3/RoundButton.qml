/*
 *   Copyright 2018 Marco Martin <mart@kde.org>
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

T.RoundButton {
    id: control

    implicitWidth: Math.max(units.gridUnit, contentItem.implicitWidth)
                            + leftPadding + rightPadding
    implicitHeight: Math.max(units.gridUnit, contentItem.implicitHeight)
                            + topPadding + bottomPadding

    leftPadding: text.length > 0 ? surfaceNormal.margins.left : contentItem.extraSpace
    topPadding: text.length > 0 ? surfaceNormal.margins.top : contentItem.extraSpace
    rightPadding: text.length > 0 ? surfaceNormal.margins.right : contentItem.extraSpace
    bottomPadding: text.length > 0 ? surfaceNormal.margins.bottom : contentItem.extraSpace

    hoverEnabled: !Kirigami.Settings.tabletMode

    contentItem: RowLayout {
        // This is the spacing which will make the icon a square inscribed in the circle with an extra smallspacing of margins
        readonly property int extraSpace: width/2 - width/2*Math.sqrt(2)/2 + units.smallSpacing
        PlasmaCore.IconItem {
            Layout.preferredWidth: units.iconSizes.smallMedium
            Layout.preferredHeight: units.iconSizes.smallMedium
            Layout.fillWidth: true
            Layout.fillHeight: true
            visible: source.length > 0
            source: control.icon ? (control.icon.name || control.icon.source) : ""
        }
        Label {
            visible: text.length > 0
            text: control.text
            font: control.font
            opacity: enabled || control.highlighted || control.checked ? 1 : 0.4
            color: theme.buttonTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    background: Item {
        opacity: control.enabled ? 1 : 0.6

        PlasmaCore.Svg {
            id: buttonSvg
            imagePath: "widgets/actionbutton"
            colorGroup: PlasmaCore.Theme.ButtonColorGroup
        }

        Private.RoundShadow {
            id: roundShadow
            visible: !control.flat || control.activeFocus || control.highlighted
            anchors.fill: parent
            state: {
                if (control.pressed) {
                    return "hidden"
                } else if (control.hovered) {
                    return "hover"
                } else if (control.activeFocus || control.highlighted) {
                    return "focus"
                } else {
                    return "shadow"
                }
            }
        }

        PlasmaCore.SvgItem {
            id: buttonItem
            svg: buttonSvg
            elementId: (control.pressed || control.checked) ? "pressed" : "normal"
            anchors.fill: parent
            //internal: if there is no hover status, don't paint on mouse over in touchscreens
            opacity: (control.pressed || control.checked || !control.flat || (roundShadow.hasOverState && control.hovered)) ? 1 : 0
            Behavior on opacity {
                PropertyAnimation { duration: units.longDuration }
            }
        }

        Private.ButtonShadow {
            anchors.fill: parent
            visible: control.text.length > 0 && (!control.flat || control.hovered) && (!control.pressed || !control.checked)
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
            opacity: control.text.length > 0 && (!control.flat || control.hovered) && (!control.pressed || !control.checked) ? 1 : 0
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
            visible: control.text.length > 0
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
