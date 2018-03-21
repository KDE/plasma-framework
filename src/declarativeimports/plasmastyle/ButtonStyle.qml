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
import QtQuick.Controls.Private 1.0 as QtQuickControlsPrivate

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents

import "private" as Private
import "private/Util.js" as Util

QtQuickControlStyle.ButtonStyle {
    id: style

    //this is the minimum size that can hold the entire contents
    property int minimumWidth
    property int minimumHeight

    label: RowLayout {
        spacing: units.smallSpacing

        property real minimumWidth: implicitWidth + style.padding.left + style.padding.right
        onMinimumWidthChanged: {
            if (control.minimumWidth !== undefined) {
                style.minimumWidth = minimumWidth;
                control.minimumWidth = minimumWidth;
            }
        }

        property real minimumHeight: implicitHeight + style.padding.top + style.padding.bottom
        onMinimumHeightChanged: {
            if (control.minimumHeight !== undefined) {
                style.minimumHeight = minimumHeight;
                control.minimumHeight = minimumHeight;
            }
        }

        PlasmaCore.IconItem {
            id: icon
            source: control.iconName || control.iconSource
            Layout.minimumWidth: valid ? units.iconSizes.tiny : 0
            Layout.preferredWidth: valid ? units.iconSizes.small : 0
            visible: valid
            Layout.minimumHeight: Layout.minimumWidth
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            active: control.hovered
            colorGroup: PlasmaCore.Theme.ButtonColorGroup
        }

        PlasmaComponents.Label {
            id: label
            Layout.alignment: Qt.AlignVCenter
            text: Util.stylizeEscapedMnemonics(Util.toHtmlEscaped(control.text))
            textFormat: Text.StyledText
            font: control.font || theme.defaultFont
            visible: control.text != ""
            Layout.fillWidth: true
            color: theme.buttonTextColor
            horizontalAlignment: icon.valid ? Text.AlignLeft : Text.AlignHCenter
            elide: Text.ElideRight
        }

        PlasmaExtras.ConditionalLoader {
            id: arrow
            when: control.menu !== null
            visible: when
            Layout.minimumWidth: units.iconSizes.small
            Layout.maximumWidth: Layout.minimumWidth
            Layout.alignment: Qt.AlignVCenter
            height: width

            source: Component {
                PlasmaCore.SvgItem {
                    visible: control.menu !== null
                    anchors.fill: parent
                    svg: PlasmaCore.Svg {
                        imagePath: "widgets/arrows"
                        colorGroup: PlasmaCore.Theme.ButtonColorGroup
                    }
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
                theme.mSize(theme.defaultFont).width*12
            }
        }

        opacity: enabled ? 1.0 : 0.5

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
