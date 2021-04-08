/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

/**
 * \internal
 */
QtQuickControlStyle.ButtonStyle {
    id: style

    //this is the minimum size that can hold the entire contents
    property int minimumWidth
    property int minimumHeight

    label: RowLayout {
        spacing: PlasmaCore.Units.smallSpacing

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
            Layout.minimumWidth: valid ? PlasmaCore.Units.iconSizes.tiny : 0
            Layout.preferredWidth: valid ? PlasmaCore.Units.iconSizes.small : 0
            visible: valid
            Layout.minimumHeight: Layout.minimumWidth
            Layout.preferredHeight: Layout.preferredWidth
            Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
            active: control.hovered
            colorGroup: PlasmaCore.Theme.ButtonColorGroup
            status: buttonSvg.hasElement("hint-focus-highlighted-background") && control.activeFocus && !control.pressed && !control.checked ? PlasmaCore.Svg.Selected : PlasmaCore.Svg.Normal
        }

        PlasmaComponents.Label {
            id: label
            Layout.fillHeight: true
            text: Util.stylizeEscapedMnemonics(Util.toHtmlEscaped(control.text))
            textFormat: Text.StyledText
            height: undefined
            font: control.font || PlasmaCore.Theme.defaultFont
            visible: control.text != ""
            Layout.fillWidth: true
            color: buttonSvg.hasElement("hint-focus-highlighted-background") && control.activeFocus && !control.pressed && !control.checked ? PlasmaCore.Theme.highlightedTextColor : PlasmaCore.Theme.buttonTextColor
            horizontalAlignment: icon.valid ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            elide: Text.ElideRight
            PlasmaCore.Svg {
                id: buttonSvg
                imagePath: "widgets/button"
            }
        }

        PlasmaExtras.ConditionalLoader {
            id: arrow
            when: control.menu !== null
            visible: when
            Layout.minimumWidth: PlasmaCore.Units.iconSizes.small
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
            prefix: control.activeFocus ? ["focus-background", "normal"] : "normal"
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
                    NumberAnimation { target: surfaceNormal; property: "opacity"; to: 1; duration: PlasmaCore.Units.shortDuration }
                    NumberAnimation { target: surfacePressed; property: "opacity"; to: 0; duration: PlasmaCore.Units.shortDuration }
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
