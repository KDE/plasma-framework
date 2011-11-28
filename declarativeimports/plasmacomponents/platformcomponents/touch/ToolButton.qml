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

Item {
    id: button

    // Commmon API
    property bool flat: true
    property bool checked: defaultAction ? defaultAction.checked : false
    property bool checkable: defaultAction ? defaultAction.checkable : false
    property alias pressed: mouse.pressed
    property alias text: label.text
    property alias iconSource: icon.source
    property alias font: label.font

    signal clicked()

    // Plasma extensiuons
    property QtObject defaultAction

    onFlatChanged: {
        if (!flat) {
            delegate.opacity = 1
        }
    }

    enabled: defaultAction==undefined||defaultAction.enabled

    implicitWidth: {
        if (label.paintedWidth == 0) {
            return implicitHeight
        } else {
            return Math.max(theme.defaultFont.mSize.width*12, icon.width + label.paintedWidth + delegate.margins.left + delegate.margins.right) + ((icon.valid) ? delegate.margins.left : 0)
        }
    }
    implicitHeight: Math.max(theme.defaultFont.mSize.height*1.6, Math.max(icon.height, label.paintedHeight) + delegate.margins.top/2 + delegate.margins.bottom/2)

    // TODO: needs to define if there will be specific graphics for
    //     disabled buttons
    opacity: enabled ? 1.0 : 0.5

    Keys.onSpacePressed: internal.pressButton()
    Keys.onReturnPressed: internal.pressButton()
    Keys.onReleased: {
        if (event.key == Qt.Key_Space ||
            event.key == Qt.Key_Return)
            internal.releaseButton()
    }

    QtObject {
        id: internal
        property bool userPressed: false

        function pressButton()
        {
            userPressed = true
        }

        function releaseButton()
        {
            userPressed = false
            if (!button.enabled) {
                return
            }

            if (defaultAction && defaultAction.checkable) {
                defaultAction.checked = !defaultAction.checked
            } else if (button.checkable) {
                button.checked = !button.checked
            }

            button.clicked()
            button.forceActiveFocus()

            if (defaultAction) {
                defaultAction.trigger()
            }
        }
    }

    Loader {
        id: delegate
        anchors.fill:parent
        property QtObject margins: item.margins
        sourceComponent: {
            if (label.paintedWidth == 0 && !flat) {
                return roundButtonComponent
            } else {
                return buttonComponent
            }
        }
    }

    Component {
        id: buttonComponent
        Item {
            anchors.fill: parent
            property alias margins: surface.margins
            ButtonShadow {
                id: shadow
                anchors.fill: parent
                visible: !flat
                state: (internal.userPressed || checked) ? "hidden" : "shadow"
            }

            PlasmaCore.FrameSvgItem {
                id: surface

                anchors.fill: parent
                imagePath: "widgets/button"
                prefix: (internal.userPressed || checked) ? "pressed" : "normal"
                //internal: if there is no hover status, don't paint on mouse over in touchscreens
                opacity: (internal.userPressed || checked || !flat || (shadow.hasOverState && mouse.containsMouse)) ? 1 : 0
                Behavior on opacity {
                    PropertyAnimation { duration: 250 }
                }
            }
        }
    }

    Component {
        id: roundButtonComponent
        Item {
            anchors.fill: parent
            property QtObject margins: QtObject {
                property int left: 8
                property int top: 8
                property int right: 8
                property int bottom: 8
            }
            RoundShadow {
                anchors.fill: parent
                state: (internal.userPressed || checked) ? "hidden" : "shadow"
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
            }
        }
    }

    Item {
        anchors {
            fill: parent
            leftMargin: delegate.margins.left
            topMargin: delegate.margins.top
            rightMargin: delegate.margins.right
            bottomMargin: delegate.margins.bottom
        }
        scale: internal.userPressed ? 0.9 : 1
        Behavior on scale {
            PropertyAnimation { duration: 100 }
        }

        IconLoader {
            id: icon

            anchors {
                verticalCenter: parent.verticalCenter
                left: label.text ? parent.left : undefined
                horizontalCenter: label.text ? undefined : parent.horizontalCenter
            }
            width: label.text ? implicitWidth : roundToStandardSize(parent.width)
            height: width
        }

        Text {
            id: label

            //FIXME: why this is needed?
            onPaintedWidthChanged: {
                icon.anchors.horizontalCenter = label.paintedWidth > 0 ? undefined : icon.parent.horizontalCenter
                icon.anchors.left = label.paintedWidth > 0 ? icon.parent.left : undefined
            }

            anchors {
                top: parent.top
                bottom: parent.bottom
                left: icon.valid ? icon.right : parent.left
                right: parent.right
            }
            font.capitalization: theme.defaultFont.capitalization
            font.family: theme.defaultFont.family
            font.italic: theme.defaultFont.italic
            font.letterSpacing: theme.defaultFont.letterSpacing
            font.pointSize: theme.defaultFont.pointSize
            font.strikeout: theme.defaultFont.strikeout
            font.underline: theme.defaultFont.underline
            font.weight: theme.defaultFont.weight
            font.wordSpacing: theme.defaultFont.wordSpacing
            color: theme.buttonTextColor
            horizontalAlignment: icon.valid ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
        }
    }

    MouseArea {
        id: mouse

        anchors.fill: parent
        hoverEnabled: true

        onPressed: internal.pressButton();

        onReleased: internal.releaseButton();
    }
}

