/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami
import "private" as Private

T.RoundButton {
    id: control

    Accessible.role: Accessible.Button

    implicitWidth: Math.max(PlasmaCore.Units.gridUnit, contentItem.implicitWidth)
                            + leftPadding + rightPadding
    implicitHeight: Math.max(PlasmaCore.Units.gridUnit, contentItem.implicitHeight)
                            + topPadding + bottomPadding

    leftPadding: text.length > 0 ? surfaceNormal.margins.left : contentItem.extraSpace
    topPadding: text.length > 0 ? surfaceNormal.margins.top : contentItem.extraSpace
    rightPadding: text.length > 0 ? surfaceNormal.margins.right : contentItem.extraSpace
    bottomPadding: text.length > 0 ? surfaceNormal.margins.bottom : contentItem.extraSpace

    hoverEnabled: !Kirigami.Settings.tabletMode

    contentItem: RowLayout {
        // This is the spacing which will make the icon a square inscribed in the circle with an extra smallspacing of margins
        readonly property int extraSpace: implicitWidth/2 - implicitWidth/2*Math.sqrt(2)/2 + PlasmaCore.Units.smallSpacing
        PlasmaCore.IconItem {
            Layout.preferredWidth: PlasmaCore.Units.iconSizes.smallMedium
            Layout.preferredHeight: PlasmaCore.Units.iconSizes.smallMedium
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
            color: PlasmaCore.Theme.buttonTextColor
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }
    }

    background: Item {
        opacity: control.enabled ? 1 : 0.6

        // Round Button

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
                PropertyAnimation { duration: PlasmaCore.Units.longDuration }
            }
        }

        // Normal Button
        // TODO: Make round button always round?

        readonly property bool useNormalButton: control.text.length > 0

        Private.ButtonShadow {
            anchors.fill: parent
            showShadow: background.useNormalButton && !control.flat && (!control.pressed || !control.checked)
        }

        PlasmaCore.FrameSvgItem {
            id: surfaceNormal
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "normal"
            opacity: background.useNormalButton && (!control.flat || control.hovered) && (!control.pressed || !control.checked) ? 1 : 0
            Behavior on opacity {
                OpacityAnimator {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }

        Private.ButtonFocus {
            anchors.fill: parent
            showFocus: background.useNormalButton && control.activeFocus && !control.pressed
        }

        Private.ButtonHover {
            anchors.fill: parent
            showHover: background.useNormalButton && control.hovered && !control.pressed
        }

        PlasmaCore.FrameSvgItem {
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "pressed"
            visible: background.useNormalButton
            opacity: control.checked || control.pressed ? 1 : 0
            Behavior on opacity {
                OpacityAnimator {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
