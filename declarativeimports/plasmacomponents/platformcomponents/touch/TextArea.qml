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
import org.kde.qtextracomponents 0.1
import "EditBubble.js" as EditBubbleHelper

Item {
    id: textArea

    // Common API
    property alias font: textEdit.font // alias to textEdit.font
    property int inputMethodHints
    property bool errorHighlight
    property alias cursorPosition: textEdit.cursorPosition
    property alias horizontalAlignment: textEdit.horizontalAlignment
    property alias verticalAlignment: textEdit.verticalAlignment
    property alias readOnly: textEdit.readOnly
    property alias selectedText: textEdit.selectedText // read-only
    property alias selectionEnd: textEdit.selectionEnd // read-only
    property alias selectionStart: textEdit.selectionStart // read-only
    property alias text: textEdit.text
    property alias textFormat: textEdit.textFormat // enumeration
    property alias wrapMode: textEdit.wrapMode // enumeration
    property string placeholderText

    property alias textField: textArea

    // functions
    function copy() {
        textEdit.copy();
    }

    function paste() {
        textEdit.paste();
    }

    function cut() {
        textEdit.cut();
    }

    function select(start, end) {
        textEdit.select(start, end);
    }

    function selectAll() {
        textEdit.selectAll();
    }

    function selectWord() {
        textEdit.selectWord();
    }

    function positionAt(pos) {
        textEdit.positionAt(pos);
    }

    function positionToRectangle(pos) {
        textEdit.positionToRectangle(pos);
    }

    // Plasma API
    property alias interactive: flickArea.interactive
    property alias contentMaxWidth: textEdit.width
    property alias contentMaxHeight: textEdit.height
    property alias textInput: textEdit

    // Set active focus to it's internal textInput.
    // Overriding QtQuick.Item forceActiveFocus function.
    function forceActiveFocus() {
        textEdit.forceActiveFocus();
    }

    // Overriding QtQuick.Item activeFocus property.
    property alias activeFocus: textEdit.activeFocus

    opacity: enabled ? 1.0 : 0.5

    TextFieldFocus {
        id: hover
        state: textArea.activeFocus ? "focus" : (mouseWatcher.containsMouse ? "hover" : "hidden")
        anchors.fill: base
    }

    MouseArea {
        id: mouseWatcher
        anchors.fill: hover
        hoverEnabled: true
    }

    PlasmaCore.FrameSvgItem {
        id: base

        // TODO: see what is the best policy for margins
        anchors {
            fill: parent
        }
        imagePath: "widgets/lineedit"
        prefix: "base"
    }

    Flickable {
        id: flickArea
        anchors {
            fill: parent
            leftMargin: 2 * base.margins.left
            rightMargin: 2 * base.margins.right + (verticalScroll.visible ? verticalScroll.width : 0)
            topMargin: 2 * base.margins.top
            bottomMargin: 2 * base.margins.bottom + (horizontalScroll.visible ? verticalScroll.width : 0)
        }
        interactive: !verticalScroll.interactive //textArea.activeFocus
        contentWidth: {
            if (textEdit.wrapMode == TextEdit.NoWrap)
                return textEdit.paintedWidth;

            return Math.min(textEdit.paintedWidth, textEdit.width);
        }
        contentHeight: Math.min(textEdit.paintedHeight, textEdit.height)
        clip: true
        MouseEventListener {
            id: mouseEventListener
            //anchors.fill: parent
            onPressed: forceActiveFocus();
            //onPressed: print(" MouseEventListener Pressed");
            onPressAndHold: {
                print(" *** MouseEventListener PressAndHold");
                //forceActiveFocus();
                EditBubbleHelper.placeEditBubble(mouse);
                editBubble.state  = (textInput.activeFocus && (textInput.selectedText != "" || textInput.canPaste)) ? "expanded" : "collapsed";
            }
            onPositionChanged: {
                EditBubbleHelper.placeEditBubble(mouse);
            }
        }

        TextEdit {
            id: textEdit
            parent: mouseEventListener
            width: flickArea.width
            height: flickArea.height
            clip: true
            wrapMode: TextEdit.Wrap
            enabled: textArea.enabled
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
            selectByMouse: verticalScroll.interactive

            onCursorPositionChanged: {
                if (cursorRectangle.x < flickArea.contentX) {
                    flickArea.contentX = cursorRectangle.x;
                    return;
                }

                if (cursorRectangle.x > flickArea.contentX +
                    flickArea.width - cursorRectangle.width) {
                    flickArea.contentX = cursorRectangle.x -
                        cursorRectangle.width;
                    return;
                }

                if (cursorRectangle.y < flickArea.contentY) {
                    flickArea.contentY = cursorRectangle.y;
                    return;
                }

                if (cursorRectangle.y > flickArea.contentY +
                    flickArea.height - cursorRectangle.height) {
                    flickArea.contentY = cursorRectangle.y -
                        cursorRectangle.height;
                    return;
                }
            }

            // Proxying keys events  is not required by the
            //     common API but is desired in the plasma API.
            Keys.onPressed: textArea.Keys.pressed(event);
            Keys.onReleased: textArea.Keys.released(event);

            Text {
                anchors.fill: parent
                text: textArea.placeholderText
                visible: textEdit.text == "" && !textArea.activeFocus
                opacity: 0.5
                color: theme.buttonTextColor
            }
        }
    }

    ScrollBar {
        id: horizontalScroll
        anchors {
            bottom: parent.bottom
            left: parent.left
            right: flickArea.right
        }
        enabled: parent.enabled
        flickableItem: flickArea
        orientation: Qt.Horizontal
        stepSize: textEdit.font.pixelSize
    }

    ScrollBar {
        id: verticalScroll
        anchors {
            right: parent.right
            top: parent.top
            bottom: flickArea.bottom
        }
        enabled: parent.enabled
        flickableItem: flickArea
        orientation: Qt.Vertical
        stepSize: textEdit.font.pixelSize
    }

    EditBubble { id: editBubble; iconSize: 32; }

    onActiveFocusChanged: {
        if (!activeFocus) {
            editBubble.state = "collapsed";
            //print("Hiding...");
        }
    }
}
