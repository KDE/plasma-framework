/*
 *   Copyright 2014 by Marco Martin <mart@kde.org>
 *   Copyright 2014 by David Edmundson <davidedmundson@kde.org>
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents

import "../private" as Private

QtQuickControlStyle.ButtonStyle {
    id: style

    property int minimumWidth
    property int minimumHeight

    label: RowLayout {
        id: buttonContent
        spacing: units.smallSpacing
        Layout.preferredHeight: Math.max(units.iconSizes.small, label.implicitHeight)

        property real minimumWidth: icon.width + label.implicitWidth + style.padding.left + style.padding.right + ((icon.valid) ? style.padding.left : 0) + (arrow.visible ? arrow.width : 0)
        onMinimumWidthChanged: {
            if (control.minimumWidth !== undefined) {
                style.minimumWidth = minimumWidth;
                control.minimumWidth = minimumWidth;
            }
        }

        property real minimumHeight: Math.max(units.iconSizes.small, label.implicitHeight) + style.padding.top + style.padding.bottom
        onMinimumHeightChanged: {
            if (control.minimumHeight !== undefined) {
                style.minimumHeight = minimumHeight;
                control.minimumHeight = minimumHeight;
            }
        }

        PlasmaCore.IconItem {
            id: icon
            //control.iconSource is an url: you pass a freedesktop icon,
            //and it inteprets it as an url in the local qml file filesystem path
            //in order to work also with upstream controls, grossly parse the url
            source: control.iconName || control.iconSource
            anchors.verticalCenter: parent.verticalCenter
            Layout.minimumWidth: valid ? parent.height: 0
            Layout.maximumWidth: Layout.minimumWidth
            visible: valid
            Layout.minimumHeight: Layout.minimumWidth
            Layout.maximumHeight: Layout.minimumWidth
            active: control.hovered
            colorGroup: PlasmaCore.Theme.ButtonColorGroup
        }

        PlasmaComponents.Label {
            id: label
            text: control.text
            font: control.font
            visible: control.text != ""
            Layout.fillWidth: true
            height: parent.height
            color: theme.buttonTextColor
            horizontalAlignment: icon.valid ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
        }

        PlasmaExtras.ConditionalLoader {
            id: arrow
            when: control.menu !== null
            visible: when
            Layout.minimumWidth: units.iconSizes.small
            Layout.maximumWidth: Layout.minimumWidth
            height: width
            anchors.verticalCenter: parent.verticalCenter

            source: Component {
                PlasmaCore.SvgItem {
                    visible: control.menu !== null
                    anchors.fill: parent
                    svg: PlasmaCore.Svg { imagePath: "widgets/arrows" }
                    elementId: "down-arrow"
                }
            }
        }
    }

    background: Item {

        implicitHeight: Math.floor(Math.max(theme.mSize(theme.defaultFont).height*1.6, style.minimumHeight))

        implicitWidth: {
            if (control.text.length == 0) {
                height;
            } else {
                Math.max(theme.mSize(theme.defaultFont).width*12, style.minimumWidth);
            }
        }

        Private.ButtonShadow {
            anchors.fill: parent
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


        //This code is duplicated here and Button and ToolButton
        //maybe we can make an AbstractButton class?
        PlasmaCore.FrameSvgItem {
            id: surfaceNormal
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "normal"
        }

        PlasmaCore.FrameSvgItem {
            id: surfacePressed
            anchors.fill: parent
            imagePath: "widgets/button"
            prefix: "pressed"
            opacity: 0
        }

        state: control.pressed || control.checked ? "pressed" : "normal"

        states: [
            State { name: "normal" },
            State { name: "pressed"
                    PropertyChanges {
                        target: surfaceNormal
                        opacity: 0
                    }
                    PropertyChanges {
                        target: surfacePressed
                        opacity: 1
                    }
            }
        ]

        transitions: [
            Transition {
                to: "normal"
                //Cross fade from pressed to normal
                ParallelAnimation {
                    NumberAnimation { target: surfaceNormal; property: "opacity"; to: 1; duration: 100 }
                    NumberAnimation { target: surfacePressed; property: "opacity"; to: 0; duration: 100 }
                }
            }
        ]

        Component.onCompleted: {
            padding.top = surfaceNormal.margins.top
            padding.left = surfaceNormal.margins.left
            padding.right = surfaceNormal.margins.right
            padding.bottom = surfaceNormal.margins.bottom
        }
    }
}
