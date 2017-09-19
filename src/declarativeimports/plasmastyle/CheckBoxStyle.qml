/*
 *   Copyright 2014 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import "private" as Private

QtQuickControlStyle.CheckBoxStyle {
    id: checkboxStyle

    label: PlasmaComponents.Label {
        text: control.text
        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.bottom
            height: 1 * units.devicePixelRatio
            color: theme.highlightColor
            visible: control.activeFocus
        }
    }
    background: Item {}

    spacing: units.smallSpacing

    indicator: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/button"
        prefix: "normal"
        width: theme.mSize(theme.defaultFont).height + margins.left
        height: theme.mSize(theme.defaultFont).height + margins.top
        opacity: control.enabled ? 1 : 0.6

        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                id: checkmarkSvg
                imagePath: "widgets/checkmarks"
            }
            elementId: "checkbox"
            opacity: {
                switch (control.checkedState) {
                case Qt.Checked:
                    return 1;
                case Qt.PartiallyChecked:
                    return 0.5;
                default:
                    return 0;
                }
            }
            anchors {
                fill: parent
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
        Private.ButtonShadow {
            z: -1
            anchors.fill: parent
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "shadow")
        }
    }
}
