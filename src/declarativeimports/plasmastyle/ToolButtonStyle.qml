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

    property int minimumWidth
    property int minimumHeight
    property bool flat: control.flat !== undefined ? control.flat : !(control.checkable && control.checked)
    property bool controlHovered: control.hovered && !(QtQuickControlsPrivate.Settings.hasTouchScreen && QtQuickControlsPrivate.Settings.isMobile)

    label: Item {
        //wrapper is needed as we are adjusting the preferredHeight of the layout from the default
        //and the implicitHeight is implicitly read only
        implicitHeight: buttonContent.Layout.preferredHeight
        implicitWidth: buttonContent.implicitWidth
        RowLayout {
            id: buttonContent
            anchors.fill: parent
            spacing: units.smallSpacing

            Layout.preferredHeight: Math.max(units.iconSizes.small, label.implicitHeight)

            property real minimumWidth: Layout.minimumWidth + style.padding.left + style.padding.right
            onMinimumWidthChanged: {
                if (control.minimumWidth !== undefined) {
                    style.minimumWidth = minimumWidth;
                    control.minimumWidth = minimumWidth;
                }
            }

            property real minimumHeight: Layout.preferredHeight + style.padding.top + style.padding.bottom
            onMinimumHeightChanged: {
                if (control.minimumHeight !== undefined) {
                    style.minimumHeight = minimumHeight;
                    control.minimumHeight = minimumHeight;
                }
            }

            PlasmaCore.IconItem {
                id: icon
                source: control.iconName || control.iconSource

                implicitHeight: label.implicitHeight
                implicitWidth: implicitHeight

                Layout.minimumWidth: valid ? parent.height: 0
                Layout.maximumWidth: Layout.minimumWidth
                visible: valid
                Layout.minimumHeight: Layout.minimumWidth
                Layout.maximumHeight: Layout.minimumWidth
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                active: style.controlHovered
                colorGroup: !flat ? PlasmaCore.Theme.ButtonColorGroup : PlasmaCore.ColorScope.colorGroup
            }

            //NOTE: this is used only to check elements existence
            PlasmaCore.FrameSvgItem {
                id: buttonsurfaceChecker
                visible: false
                imagePath: "widgets/button"
                prefix: style.flat ? ["toolbutton-hover", "normal"] : "normal"
            }

            PlasmaComponents.Label {
                id: label
                Layout.minimumWidth: implicitWidth
                Layout.fillHeight: true
                height: undefined
                text: Util.stylizeEscapedMnemonics(Util.toHtmlEscaped(control.text))
                textFormat: Text.StyledText
                font: control.font || theme.defaultFont
                visible: control.text != ""
                Layout.fillWidth: true
                color: (controlHovered || !flat) && buttonsurfaceChecker.usedPrefix != "toolbutton-hover" ? theme.buttonTextColor : PlasmaCore.ColorScope.textColor
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

                Layout.minimumHeight: Layout.minimumWidth
                Layout.maximumHeight: Layout.maximumWidth

                Layout.alignment: Qt.AlignVCenter

                source: Component {
                    PlasmaCore.SvgItem {
                        visible: control.menu !== null
                        svg: PlasmaCore.Svg {
                            imagePath: "widgets/arrows"
                            colorGroup: (style.controlHovered || !style.flat) && buttonsurfaceChecker.usedPrefix != "toolbutton-hover" ? PlasmaCore.Theme.ButtonColorGroup : PlasmaCore.ColorScope.colorGroup
                        }
                        elementId: "down-arrow"
                    }
                }
            }
        }
    }

    background: {
        if (control.text.length == 0 && (control.parent && (control.parent.spacing === undefined || control.parent.spacing !== 0)) && !style.flat && !control.menu) {
            return roundButtonComponent
        } else {
            return buttonComponent
        }
    }

    Component {
        id: roundButtonComponent
        Item {
            id: roundButtonDelegate
            implicitHeight: Math.floor(theme.mSize(theme.defaultFont).height*1.6)
            implicitWidth: implicitHeight

            property QtObject margins: QtObject {
                property int left: control.width/8
                property int top: control.width/8
                property int right: control.width/8
                property int bottom: control.width/8
            }
            property alias hasOverState: roundShadow.hasOverState
            Private.RoundShadow {
                id: roundShadow
                visible: !style.flat || control.activeFocus
                anchors.fill: parent
                state: {
                    if (control.pressed) {
                        return "hidden"
                    } else if (style.controlHovered) {
                        return "hover"
                    } else if (control.activeFocus) {
                        return "focus"
                    } else {
                        return "shadow"
                    }
                }
            }

            PlasmaCore.Svg {
                id: buttonSvg
                imagePath: "widgets/actionbutton"
            }

            PlasmaCore.SvgItem {
                id: buttonItem
                svg: buttonSvg
                elementId: (control.pressed || control.checked) ? "pressed" : "normal"
                width: Math.floor(parent.height/2) * 2
                height: width
                anchors.centerIn: parent
                //internal: if there is no hover status, don't paint on mouse over in touchscreens
                opacity: (control.pressed || control.checked || !style.flat || (roundShadow.hasOverState && style.controlHovered)) ? 1 : 0
                Behavior on opacity {
                    PropertyAnimation { duration: units.longDuration }
                }
            }
        }
    }

    Component {
        id: buttonComponent
        Item {
            id: buttonSurface
            implicitHeight: Math.floor(theme.mSize(theme.defaultFont).height*1.6)

            implicitWidth: {
                if (control.text.length == 0) {
                    implicitHeight;
                } else {
                    Math.floor(theme.mSize(theme.defaultFont).width*12);
                }
            }
            Connections {
                target: control
                onHoveredChanged: {
                    if (style.controlHovered) {
                        control.z += 2
                    } else {
                        control.z -= 2
                    }
                }
            }

            Private.ButtonShadow {
                id: shadow
                visible: !style.flat || control.activeFocus
                anchors.fill: parent
                enabledBorders: surfaceNormal.enabledBorders
                state: {
                    if (control.pressed) {
                        return "hidden"
                    } else if (style.controlHovered) {
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
                prefix: style.flat ? ["toolbutton-hover", "normal"] : "normal"

                enabledBorders: {
                    if (style.flat || !control.parent ||
                        control.parent.width < control.parent.implicitWidth ||
                        control.parent.spacing !== 0 ||
                        !bordersSvg.hasElement("pressed-hint-compose-over-border")) {
                        if (shadows !== null) {
                            shadows.destroy()
                        }
                        return "AllBorders"
                    }

                    var borders = new Array()
                    if (control.x == 0) {
                        borders.push("LeftBorder")
                        shadow.anchors.leftMargin = 0;
                    } else {
                        shadow.anchors.leftMargin = -1;
                    }
                    if (control.y == 0) {
                        borders.push("TopBorder")
                        shadow.anchors.topMargin = 0;
                    } else {
                        shadow.anchors.topMargin = -1;
                    }
                    if (control.x + control.width >= control.parent.width) {
                        borders.push("RightBorder")
                        shadow.anchors.rightMargin = 0;
                    } else {
                        shadow.anchors.rightMargin = -1;
                    }
                    if (control.y + control.height >= control.parent.height) {
                        borders.push("BottomBorder")
                        shadow.anchors.bottomMargin = 0;
                    } else {
                        shadow.anchors.bottomMargin = -1;
                    }

                    if (shadows === null) {
                        shadows = shadowsComponent.createObject(buttonSurface)
                    }

                    return borders.join("|")
                }

                PlasmaCore.Svg {
                    id: bordersSvg
                    imagePath: "widgets/button"
                }

            }

            PlasmaCore.FrameSvgItem {
                id: surfacePressed
                anchors.fill: parent
                imagePath: "widgets/button"
                prefix: style.flat ? ["toolbutton-pressed", "pressed"] : "pressed"
                enabledBorders: surfaceNormal.enabledBorders
                opacity: 0
            }

            property Item shadows
            Component {
                id: shadowsComponent
                Item {
                    anchors.fill: parent

                    PlasmaCore.SvgItem {
                        svg: bordersSvg
                        width: naturalSize.width
                        elementId: (buttonSurface.state == "pressed" ? surfacePressed.prefix : surfaceNormal.prefix) + "-left"
                        visible: button.x > 0
                        anchors {
                            left: parent.left
                            top: parent.top
                            bottom: parent.bottom
                            margins: 1
                            leftMargin: -1
                        }
                    }
                    PlasmaCore.SvgItem {
                        svg: bordersSvg
                        width: naturalSize.width
                        elementId: (buttonSurface.state == "pressed" ? surfacePressed.prefix : surfaceNormal.prefix) + "-right"
                        visible: button.x + button.width < button.parent.width
                        anchors {
                            right: parent.right
                            top: parent.top
                            bottom: parent.bottom
                            margins: 1
                            rightMargin: -1
                        }
                    }
                    PlasmaCore.SvgItem {
                        svg: bordersSvg
                        height: naturalSize.height
                        elementId: (buttonSurface.state == "pressed" ? surfacePressed.prefix : surfaceNormal.prefix) + "-top"
                        visible: button.y > 0
                        anchors {
                            left: parent.left
                            top: parent.top
                            right: parent.right
                            margins: 1
                            topMargin: -1
                        }
                    }
                    PlasmaCore.SvgItem {
                        svg: bordersSvg
                        height: naturalSize.height
                        elementId: (buttonSurface.state == "pressed" ? surfacePressed.prefix : surfaceNormal.prefix) + "-bottom"
                        visible: button.y + button.height < button.parent.height
                        anchors {
                            left: parent.left
                            right: parent.right
                            bottom: parent.bottom
                            margins: 1
                            bottomMargin: -1
                        }
                    }
                }
            }

            state: (control.pressed || control.checked ? "pressed" : (style.controlHovered ? "hover" : "normal"))

            states: [
                State { name: "normal"
                    PropertyChanges {
                        target: surfaceNormal
                        opacity: style.flat ? 0 : 1
                    }
                    PropertyChanges {
                        target: surfacePressed
                        opacity: 0
                    }
                },
                State { name: "hover"
                    PropertyChanges {
                        target: surfaceNormal
                        opacity: 1
                    }
                    PropertyChanges {
                        target: surfacePressed
                        opacity: 0
                    }
                },
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
                    //Cross fade from pressed to normal
                    ParallelAnimation {
                        NumberAnimation { target: surfaceNormal; property: "opacity"; duration: 100 }
                        NumberAnimation { target: surfacePressed; property: "opacity"; duration: 100 }
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
}
