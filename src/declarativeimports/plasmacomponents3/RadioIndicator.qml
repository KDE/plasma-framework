/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as P

Item {
    id: root

    required property T.AbstractButton control

    property size hintSize: radioButtonSvg.fromCurrentTheme && radioButtonSvg.hasElement("hint-size")
        ? radioButtonSvg.elementSize("hint-size")
        : Qt.size(PlasmaCore.Units.iconSizes.small, PlasmaCore.Units.iconSizes.small)

    implicitWidth: hintSize.width
    implicitHeight: hintSize.height
    opacity: control.enabled ? 1 : 0.5
    layer.enabled: opacity < 1

    PlasmaCore.Svg {
        id: radioButtonSvg
        imagePath: "widgets/radiobutton"
    }

    Loader {
        anchors.fill: parent
        sourceComponent: radioButtonSvg.fromCurrentTheme
            // Hardcode breeze-light and breeze-dark because fromCurrentTheme is
            // false for them. This is because they don't contain any SVGs and
            // inherit all of them from the default theme.
            || PlasmaCore.Theme.themeName === "breeze-light"
            || PlasmaCore.Theme.themeName === "breeze-dark"
            ? radiobuttonComponent : compatibilityComponent
    }

    // Uses newer radiobutton.svg
    Component {
        id: radiobuttonComponent
        PlasmaCore.SvgItem {
            svg: radioButtonSvg
            elementId: "normal"
            anchors.centerIn: parent
            implicitWidth: naturalSize.width
            implicitHeight: naturalSize.height
            PlasmaCore.SvgItem {
                z: -1
                svg: radioButtonSvg
                elementId: "shadow"
                anchors.centerIn: parent
                implicitWidth: naturalSize.width
                implicitHeight: naturalSize.height
                visible: opacity > 0
                opacity: enabled && !root.control.down
                Behavior on opacity {
                    enabled: root.control.down
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }
            PlasmaCore.SvgItem {
                svg: radioButtonSvg
                elementId: "checked"
                anchors.centerIn: parent
                implicitWidth: naturalSize.width
                implicitHeight: naturalSize.height
                visible: opacity > 0
                opacity: (root.control.checked || root.control.down)
                    && !(root.control instanceof T.ItemDelegate && root.control.highlighted)
                Behavior on opacity {
                    enabled: root.control.checked || root.control.down
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }
            PlasmaCore.SvgItem {
                svg: radioButtonSvg
                elementId: "focus"
                anchors.centerIn: parent
                implicitWidth: naturalSize.width
                implicitHeight: naturalSize.height
                visible: opacity > 0
                opacity: root.control.visualFocus
                Behavior on opacity {
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }
            PlasmaCore.SvgItem {
                svg: radioButtonSvg
                elementId: "hover"
                anchors.centerIn: parent
                implicitWidth: naturalSize.width
                implicitHeight: naturalSize.height
                visible: opacity > 0
                opacity: root.control.hovered
                Behavior on opacity {
                    enabled: root.control.hovered
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }
            PlasmaCore.SvgItem {
                svg: radioButtonSvg
                elementId: "symbol"
                anchors.centerIn: parent
                implicitWidth: naturalSize.width
                implicitHeight: naturalSize.height
                visible: scale > 0
                scale: root.control.checked
                Behavior on scale {
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.OutCubic
                    }
                }
            }
        }
    }

    // Uses older combination of actionbutton.svg and checkmarks.svg.
    // NOTE: Do not touch this except to fix bugs. This is for compatibility.
    Component {
        id: compatibilityComponent
        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                id: buttonSvg
                imagePath: "widgets/actionbutton"
            }
            elementId: "normal"

            anchors.centerIn: parent
            implicitWidth: implicitHeight
            implicitHeight: PlasmaCore.Units.iconSizes.small

            PlasmaCore.SvgItem {
                id: checkmark
                svg: PlasmaCore.Svg {
                    id: checkmarksSvg
                    imagePath: "widgets/checkmarks"
                }
                elementId: "radiobutton"
                opacity: root.control.checked ? 1 : 0
                anchors {
                    fill: parent
                }
                Behavior on opacity {
                    NumberAnimation {
                        duration: PlasmaCore.Units.longDuration
                        easing.type: Easing.InOutQuad
                    }
                }
            }
            P.RoundShadow {
                anchors.fill: parent
                z: -1
                state: root.control.activeFocus ? "focus" : (root.control.hovered ? "hover" : "shadow")
            }
        }
    }
}
