/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: textField

    // Common API
    property bool errorHighlight: false // TODO
    property string placeholderText
    property alias inputMethodHints: textInput.inputMethodHints
    property alias font: textInput.font

    property alias cursorPosition: textInput.cursorPosition
    property alias readOnly: textInput.readOnly
    property alias echoMode: textInput.echoMode // Supports TextInput.Normal,TextInput.Password, TextInput.NoEcho, TextInput.PasswordEchoOnEdit
    property alias acceptableInput: textInput.acceptableInput // read-only
    property alias inputMask: textInput.inputMask
    property alias validator: textInput.validator
    property alias selectedText: textInput.selectedText // read-only
    property alias selectionEnd: textInput.selectionEnd // read-only
    property alias selectionStart: textInput.selectionStart // read-only
    property alias text: textInput.text
    property alias maximumLength: textInput.maximumLength

    function copy() {
        textInput.copy();
    }

    function paste() {
        textInput.paste();
    }

    function cut() {
        textInput.cut();
    }

    function select(start, end) {
        textInput.select(start, end);
    }

    function selectAll() {
        textInput.selectAll();
    }

    function selectWord() {
        textInput.selectWord();
    }

    function positionAt(pos) {
        textInput.positionAt(pos);
    }

    function positionToRectangle(pos) {
        textInput.positionToRectangle(pos);
    }


    // Set active focus to it's internal textInput.
    // Overriding QtQuick.Item forceActiveFocus function.
    function forceActiveFocus() {
        textInput.forceActiveFocus();
    }

    // Overriding QtQuick.Item activeFocus property.
    property alias activeFocus: textInput.activeFocus

    // TODO: fix default size
    implicitWidth: 100
    implicitHeight: 26
    // TODO: needs to define if there will be specific graphics for
    //     disabled text fields
    opacity: enabled ? 1.0 : 0.5

    TextFieldFocus {
        id: hover
        state: textInput.activeFocus ? "focus" : (mouseWatcher.containsMouse ? "hover" : "hidden")
        anchors.fill: base
    }

    PlasmaCore.FrameSvgItem {
        id: base

        // TODO: see what is the correct policy for margins
        anchors.fill: parent
        imagePath: "widgets/lineedit"
        prefix: "base"
    }

    MouseArea {
        id: mouseWatcher
        anchors.fill: hover
        hoverEnabled: true
    }

    Text {
        anchors.fill: textInput
        text: placeholderText
        visible: textInput.text == "" && !textField.activeFocus
        // XXX: using textColor and low opacity for theming placeholderText
        color: theme.viewTextColor
        opacity: 0.5
        elide: Text.ElideRight
        clip: true
        font.capitalization: theme.defaultFont.capitalization
        font.family: theme.defaultFont.family
        font.italic: theme.defaultFont.italic
        font.letterSpacing: theme.defaultFont.letterSpacing
        font.pointSize: theme.defaultFont.pointSize
        font.strikeout: theme.defaultFont.strikeout
        font.underline: theme.defaultFont.underline
        font.weight: theme.defaultFont.weight
        font.wordSpacing: theme.defaultFont.wordSpacing
    }

    TextInput {
        id: textInput

        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            // TODO: see what is the correct policy for margins
            leftMargin: 2 * base.margins.left
            rightMargin: 2 * base.margins.right
        }
        selectByMouse: true
        y: (height - font.pixelSize) * 0.4 // XXX: verticalCenter anchor is not centering the text
        height: Math.min(2 * font.pixelSize, parent.height)
        color: theme.textColor
        enabled: textField.enabled
        clip: true

        // Proxying keys events  is not required by the
        //     common API but is desired in the plasma API.
        Keys.onPressed: textField.Keys.pressed(event);
        Keys.onReleased: textField.Keys.released(event);
    }
}