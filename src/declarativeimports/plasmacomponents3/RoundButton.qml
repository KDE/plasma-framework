/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.ksvg 1.0 as KSvg
//NOTE: importing PlasmaCore is necessary in order to make KSvg load the current Plasma Theme
import org.kde.plasma.core 2 as PlasmaCore
import org.kde.kirigami 2 as Kirigami
import "private" as Private

T.RoundButton {
    id: control

    Accessible.role: Accessible.Button

    implicitWidth: Math.max(Kirigami.Units.gridUnit, implicitContentWidth)
                            + leftPadding + rightPadding
    implicitHeight: Math.max(Kirigami.Units.gridUnit, implicitContentHeight)
                            + topPadding + bottomPadding

    topPadding: text.length > 0 ? surfaceNormal.margins.top : __extraSpace
    leftPadding: text.length > 0 ? surfaceNormal.margins.left : __extraSpace
    rightPadding: text.length > 0 ? surfaceNormal.margins.right : __extraSpace
    bottomPadding: text.length > 0 ? surfaceNormal.margins.bottom : __extraSpace

    hoverEnabled: !Kirigami.Settings.tabletMode

    Kirigami.MnemonicData.enabled: control.enabled && control.visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: control.text

    Kirigami.Theme.colorSet: Kirigami.Theme.Button
    Kirigami.Theme.inherit: false

    // This is the spacing which will make the icon a square inscribed in the circle with an extra smallspacing of margins
    readonly property int __extraSpace: implicitContentWidth * (1 - Math.sqrt(2) / 2) / 2 + Kirigami.Units.smallSpacing

    icon.width: Kirigami.Units.iconSizes.smallMedium
    icon.height: Kirigami.Units.iconSizes.smallMedium

    contentItem: Private.ButtonContent {
        labelText: control.Kirigami.MnemonicData.richTextLabel
    }

    background: Item {
        opacity: control.enabled ? 1 : 0.6

        // Round Button

        KSvg.Svg {
            id: buttonSvg
            imagePath: "widgets/actionbutton"
            colorSet: KSvg.Svg.Button
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

        KSvg.SvgItem {
            id: buttonItem
            svg: buttonSvg
            elementId: (control.pressed || control.checked) ? "pressed" : "normal"
            anchors.fill: parent
            //internal: if there is no hover status, don't paint on mouse over in touchscreens
            opacity: (control.pressed || control.checked || !control.flat || (roundShadow.hasOverState && control.hovered)) ? 1 : 0
            Behavior on opacity {
                PropertyAnimation { duration: Kirigami.Units.longDuration }
            }
        }

        // Normal Button
        // TODO: Make round button always round?

        readonly property bool useNormalButton: control.text.length > 0

        Private.ButtonShadow {
            anchors.fill: parent
            showShadow: background.useNormalButton && !control.flat && (!control.pressed || !control.checked)
        }

        KSvg.FrameSvgItem {
            id: surfaceNormal
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "normal"
            opacity: background.useNormalButton && (!control.flat || control.hovered) && (!control.pressed || !control.checked) ? 1 : 0
            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.longDuration
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

        KSvg.FrameSvgItem {
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "pressed"
            visible: background.useNormalButton
            opacity: control.checked || control.pressed ? 1 : 0
            Behavior on opacity {
                OpacityAnimator {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
}
