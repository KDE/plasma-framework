/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2011 by Marco Martin <mart@kde.org>
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import "private" as Private

/**
 * A plasma theme based toolbutton.
 */
Item {
    id: button

    // Commmon API
    /**
     * true if the button is flat.
     */
    property bool flat: true

    /**
     * true if the button is checked.
     */
    property bool checked: defaultAction ? defaultAction.checked : false

    /**
     * true if the button is checkable.
     */
    property bool checkable: defaultAction ? defaultAction.checkable : false

    /**
     * type:string
     * true if the button is currently pressed.
     */
    property alias pressed: mouse.pressed

    /**
     * type:string
     * The text of the button
     */
    property alias text: label.text

    /**
     * type:variant
     *
     * Sets the icon for the button. It can be any image from any protocol
     * supported by the Image element, or a freedesktop-compatible icon name
     */
    property alias iconSource: icon.source

    /**
     * type:font
     * The font for the button
     */
    property alias font: label.font

    /**
     * Emited when the button is clicked.
     */
    signal clicked()

    // Plasma extensiuons
    property QtObject defaultAction


    enabled: defaultAction == undefined || defaultAction.enabled

    /**
     * The smallest width this button can be to show all the contents
     */
    //icon + label + left margin + right margin + spacing between icon and text
    //here it assumesleft margin = right top = bottom, why?
    // because the right and bottom margins can be disabled, so they would return 0, but their actual size is still needed for size hints
    property real minimumWidth: theme.smallIconSize + label.paintedWidth + delegate.margins.left + delegate.margins.left + ((icon.valid) ? delegate.margins.left : 0)

    /**
     * The smallest height this button can be to show all the contents
     */
    property real minimumHeight: Math.max(theme.smallIconSize, label.paintedHeight) + delegate.margins.top + delegate.margins.top

    implicitWidth: {
        if (label.text.length == 0) {
            height;
        } else {
            Math.max(theme.defaultFont.mSize.width*12, minimumWidth);
        }
    }

    implicitHeight: Math.max(theme.defaultFont.mSize.height*1.6, minimumHeight)

    // TODO: needs to define if there will be specific graphics for
    //     disabled buttons
    opacity: enabled ? 1.0 : 0.5

    Keys.onSpacePressed: internal.userPressed = true
    Keys.onReturnPressed: internal.userPressed = true
    Keys.onReleased: {
        internal.userPressed = false
        if (event.key == Qt.Key_Space ||
            event.key == Qt.Key_Return)
            internal.clickButton()
    }

    onActiveFocusChanged: {
        if (activeFocus) {
            shadow.state = "focus"
        } else if (checked) {
            shadow.state = "hidden"
        } else {
            shadow.state = "shadow"
        }
    }

    QtObject {
        id: internal
        property bool userPressed: false

        function clickButton()
        {
            if (!button.enabled) {
                return
            }

            if (defaultAction && defaultAction.checkable) {
                defaultAction.checked = !defaultAction.checked
            } else if (button.checkable) {
                button.checked = !button.checked
            }

            if (button.KeyNavigation.tab || button.KeyNavigation.backtab) {
                // Only focus the button if it is set up for keyboard
                // navigation. This avoid getting a strange focus frame around
                // buttons which are usually not focusable, such as buttons in
                // a toolbar.
                button.forceActiveFocus();
            }
            button.clicked()

            if (defaultAction) {
                defaultAction.trigger()
            }
        }
    }

    Loader {
        id: delegate
        anchors.fill: parent
        property QtObject margins: item.margins
        property string shadowState: "shadow"
        sourceComponent: {
            if (label.text.length == 0 && button.width == button.height && (button.parent && button.parent.checkedButton === undefined) && !flat) {
                return roundButtonComponent
            } else {
                return buttonComponent
            }
        }
    }

    Component {
        id: buttonComponent
        Item {
            parent: delegate
            anchors.fill: parent
            property alias margins: surface.margins
            property alias hasOverState: shadow.hasOverState
            Private.ButtonShadow {
                id: shadow
                anchors.fill: parent
                visible: !flat && (surface.enabledBorders == "AllBorders" || state == "hover" || state == "focus")
                state: delegate.shadowState
            }

            PlasmaCore.FrameSvgItem {
                id: surface

                enabledBorders: {
                    if (flat ||
                        button.parent.width < button.parent.implicitWidth ||
                        button.parent.checkedButton === undefined ||
                        !bordersSvg.hasElement("pressed-hint-compose-over-border")) {
                        if (shadows !== null) {
                            shadows.destroy()
                        }
                        return "AllBorders"
                    }

                    var borders = new Array()
                    if (button.x == 0) {
                        borders.push("LeftBorder")
                    }
                    if (button.y == 0) {
                        borders.push("TopBorder")
                    }
                    if (button.x + button.width >= button.parent.width) {
                        borders.push("RightBorder")
                    }
                    if (button.y + button.height >= button.parent.height) {
                        borders.push("BottomBorder")
                    }

                    if (shadows === null) {
                        shadows = shadowsComponent.createObject(surface)
                    }

                    return borders.join("|")
                }

                anchors.fill: parent
                imagePath: "widgets/button"
                prefix: (internal.userPressed || checked) ? "pressed" : "normal"
                //internal: if there is no hover status, don't paint on mouse over in touchscreens
                opacity: (internal.userPressed || checked || !flat || (shadow.hasOverState && mouse.containsMouse && button.enabled)) ? 1 : 0
                Behavior on opacity {
                    PropertyAnimation { duration: 250 }
                }

                PlasmaCore.Svg {
                    id: bordersSvg
                    imagePath: "widgets/button"
                }

                property Item shadows
                Component {
                    id: shadowsComponent
                    Item {
                        anchors.fill: parent

                        PlasmaCore.SvgItem {
                            svg: bordersSvg
                            width: naturalSize.width
                            elementId: surface.prefix+"-left"
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
                            elementId: surface.prefix+"-right"
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
                            elementId: surface.prefix+"-top"
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
                            width: naturalSize.width
                            elementId: surface.prefix+"-bottom"
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
            }
        }
    }

    Component {
        id: roundButtonComponent
        Item {
            id: roundButtonDelegate
            parent: delegate
            anchors.fill: parent
            property QtObject margins: QtObject {
                property int left: delegate.width/8
                property int top: delegate.width/8
                property int right: delegate.width/8
                property int bottom: delegate.width/8
            }
            property alias hasOverState: roundShadow.hasOverState
            Private.RoundShadow {
                id: roundShadow
                visible: !flat
                anchors.fill: parent
                state: delegate.shadowState
            }

            PlasmaCore.Svg {
                id: buttonSvg
                imagePath: "widgets/actionbutton"
            }

            PlasmaCore.SvgItem {
                id: buttonItem
                svg: buttonSvg
                elementId: (internal.userPressed || checked) ? "pressed" : "normal"
                width: parent.height
                height: width
                //internal: if there is no hover status, don't paint on mouse over in touchscreens
                opacity: (internal.userPressed || checked || !flat || (roundShadow.hasOverState && mouse.containsMouse)) ? 1 : 0
                Behavior on opacity {
                    PropertyAnimation { duration: 250 }
                }
            }
        }
    }

    Row {
        anchors {
            fill: parent
            leftMargin: delegate.margins.left
            topMargin: delegate.margins.top
            rightMargin: delegate.margins.right
            bottomMargin: delegate.margins.bottom
        }

        spacing: icon.valid ? delegate.margins.left : 0

        PlasmaCore.IconItem {
            id: icon
            anchors.verticalCenter: parent.verticalCenter
            width: valid ? Math.min(parent.width, parent.height): 0
            height: width
            active: delegate.item.hasOverState && mouse.containsMouse
        }

        Text {
            id: label

            width: parent.width - icon.width - parent.spacing
            height: parent.height

            font.capitalization: theme.defaultFont.capitalization
            font.family: theme.defaultFont.family
            font.italic: theme.defaultFont.italic
            font.letterSpacing: theme.defaultFont.letterSpacing
            font.pointSize: theme.defaultFont.pointSize
            font.strikeout: theme.defaultFont.strikeout
            font.underline: theme.defaultFont.underline
            font.weight: theme.defaultFont.weight
            font.wordSpacing: theme.defaultFont.wordSpacing

            color: mouse.containsMouse ? theme.buttonTextColor : theme.textColor
            Behavior on color { ColorAnimation { duration: 100 } }

            horizontalAlignment: icon.valid ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: delegate.item.hasOverState

        onPressed: internal.userPressed = true
        onReleased: internal.userPressed = false
        onCanceled: {
            internal.userPressed = false
            delegate.shadowState = "shadow"
        }
        onClicked: internal.clickButton()

        onEntered: {
            if (delegate.item.hasOverState && !flat && !internal.userPressed && !checked) {
                delegate.shadowState = "hover"
            }
            button.z += 2
        }
        onExited: {
            if (!flat) {
                if (button.activeFocus) {
                    delegate.shadowState = "focus"
                } else if (checked) {
                    delegate.shadowState = "hidden"
                } else {
                    delegate.shadowState = "shadow"
                }
            }
            button.z -= 2
        }
    }
}

