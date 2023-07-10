/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.ksvg 1.0 as KSvg
import org.kde.kirigami 2.20 as Kirigami
import "private" as Private

Item {
    id: root

    required property T.AbstractButton control

    implicitWidth: inactive.implicitWidth
    implicitHeight: Math.max(inactive.implicitHeight, button.implicitHeight)
    opacity: control.enabled ? 1 : 0.6

    KSvg.Svg {
        id: switchSvg
        imagePath: "widgets/switch"
        // FIXME
        colorSet: control.Kirigami.Theme.colorSet
    }

    KSvg.FrameSvgItem {
        id: inactive
        anchors {
            left: parent.left
            right: parent.right
            leftMargin: 1
            rightMargin: 1
            verticalCenter: parent.verticalCenter
        }
        implicitHeight: switchSvg.hasElement("hint-bar-size")
                ? switchSvg.elementSize("hint-bar-size").height
                : button.implicitHeight
        implicitWidth: switchSvg.hasElement("hint-bar-size")
                ? switchSvg.elementSize("hint-bar-size").width
                : root.implicitHeight * 2
        imagePath: "widgets/switch"
        prefix: "inactive"
    }
    KSvg.FrameSvgItem {
        anchors.fill: inactive
        imagePath: "widgets/switch"
        prefix: "active"
        opacity: root.control.checked ? 1 : 0
        Behavior on opacity {
            OpacityAnimator {
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
    }
    KSvg.SvgItem {
        id: button
        x: root.control.mirrored ? (root.control.checked ? 0 : parent.width - width) : (root.control.checked ? parent.width - width : 0)
        anchors.verticalCenter: parent.verticalCenter
        svg: switchSvg
        elementId: control.pressed ? "handle-pressed" : (control.hovered || control.focus ? "handle-hover" : "handle")
        implicitWidth: naturalSize.width
        implicitHeight: naturalSize.height

        Behavior on x {
            XAnimator {
                duration: Kirigami.Units.shortDuration
                easing.type: Easing.InOutQuad
            }
        }
        KSvg.SvgItem {
            svg: switchSvg
            z: -1
            anchors.centerIn: parent
            implicitWidth: naturalSize.width
            implicitHeight: naturalSize.height
            elementId: "handle-shadow"
            visible: enabled && !control.pressed
        }
        KSvg.SvgItem {
            anchors.centerIn: parent
            implicitWidth: naturalSize.width
            implicitHeight: naturalSize.height
            svg: switchSvg
            elementId: "handle-focus"
            visible: opacity > 0
            opacity: control.visualFocus
            Behavior on opacity {
                NumberAnimation {
                    duration: Kirigami.Units.longDuration
                    easing.type: Easing.OutCubic
                }
            }
        }
    }
}

