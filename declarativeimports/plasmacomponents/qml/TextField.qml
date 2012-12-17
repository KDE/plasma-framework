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
import "private" as Private

/**
 * A plasma theme based text field widget.
 */
FocusScope {
    id: textField

    // Common API
    /**
     * Whether the text field is highlighted or not
     *
     * If it is true then the problematic lines will be highlighted.
     *
     * @warning Not implemented in Plasma components.
     */
    property bool errorHighlight: false // TODO

    /**
     * type:string
     *
     * The text displayed in when the text is empty.
     *
     * The default value is an empty string, meaning no placeholderText shown.
     */
    property string placeholderText

    /**
     * type:int
     * The currently supported input method hints for the text field.
     *
     * The default values is Qt.ImhNone.
     */
    property alias inputMethodHints: textInput.inputMethodHints

    /**
     * type:font
     * The font used in the text field.
     *
     * The default font value is the font from plasma desktop theme.
     */
    property alias font: textInput.font

    /**
     * type:int
     * This property holds the current cursor position.
     */
    property alias cursorPosition: textInput.cursorPosition

    /**
     * type:bool
     * This property holds if the text field can be modified by the user interaction.
     *
     * The default value is false.
     */
    property alias readOnly: textInput.readOnly

    /**
     * type:enum
     *
     * Specifies how the text should be displayed in the TextField.
     *
     * The acceptable values are:
     * - TextInput.Normal: Displays the text as it is. (Default)
     * - TextInput.Password: Displays asterixes instead of characters.
     * - TextInput.NoEcho: Displays nothing.
     * - TextInput.PasswordEchoOnEdit: Displays all but the current character
     *   as asterixes.
     *
     * The default value is TextInput.Normal
     */
    property alias echoMode: textInput.echoMode


    property alias passwordCharacter: textInput.passwordCharacter
    property alias acceptableInput: textInput.acceptableInput // read-only

    /**
     * type:string
     *
     * Allows you to set an input mask on the TextField, restricting the
     * allowable text inputs.
     *
     * See QLineEdit::inputMask for further details, as the exact same mask
     * strings are used by TextInput.
     */
    property alias inputMask: textInput.inputMask

    /**
     * type:QtObject
     *
     * Allows you to set a validator on the TextField. When a validator is set
     * the TextField will only accept input which leaves the text property in
     * an acceptable or intermediate state.  The accepted signal will only be
     * sent if the text is in an acceptable state when enter is pressed.
     *
     * Currently supported validators are IntValidator, DoubleValidator and
     * RegExpValidator.
     *
     * An example of using validators is shown below, which allows input of
     * integers between 11 and 31 into the text input:
     *
     * @code
     * import QtQuick 1.0
     * TextInput {
     *     validator: IntValidator { bottom: 11; top: 31 }
     *     focus: true
     * }
     * @endcode
     */
    property alias validator: textInput.validator

    /**
     * type:string
     * The text selected by the user. If no text is selected it holds an empty string.
     *
     * This property is read-only.
     */
    property alias selectedText: textInput.selectedText

    /**
     * type:int
     * The cursor position after the last character in the current selection.
     *
     * This property is read-only.
     */
    property alias selectionEnd: textInput.selectionEnd

    /**
     * type:int
     * The cursor position before the first character in the current selection.
     *
     * This property is read-only.
     */
    property alias selectionStart: textInput.selectionStart

    /**
     * type:string
     * The text in the text field.
     */
    property alias text: textInput.text

    /**
     * type:int
     *
     * The maximum permitted length of the text in the TextField.  If the text
     * is too long, it is truncated at the limit.
     *
     * Default value is 32767.
     */
    property alias maximumLength: textInput.maximumLength

    /**
     * Emitted when the text input is accepted.
     */
    signal accepted()

    //Plasma api
    /**
     * Whether the button to clear the text from TextField is visible.
     */
    property bool clearButtonShown: false

    /**
     * Copies the currently selected text to the system clipboard.
     */
    function copy() {
        textInput.copy();
    }

    /**
     * Replaces the currently selected text by the contents of the system
     * clipboard.
     */
    function paste() {
        textInput.paste();
    }

    /**
     * Moves the currently selected text to the system clipboard.
     */
    function cut() {
        textInput.cut();
    }

    /**
     * Causes the text from start to end to be selected.
     *
     * If either start or end is out of range, the selection is not changed.
     * After calling this, selectionStart will become the lesser and
     * selectionEnd will become the greater (regardless of the order passed to
     * this method).
     *
     * @param int start Start of selection
     * @param int end End of selection
     */
    function select(start, end) {
        textInput.select(start, end);
    }

    /**
     * Causes all text to be selected.
     */
    function selectAll() {
        textInput.selectAll();
    }

    /**
     * Causes the word closest to the current cursor position to be selected.
     */
    function selectWord() {
        textInput.selectWord();
    }

    /**
     * This function returns the character position at x pixels from the left
     * of the TextField.
     *
     * Position 0 is before the first character, position 1 is after the first
     * character but before the second, and so on until position text.length,
     * which is after all characters.  This means that for all x values before
     * the first character this function returns 0, and for all x values after
     * the last character this function returns text.length.
     *
     * @param int pos x-coordinate we are interested in.
     * @return int the character position
     */
    function positionAt(pos) {
        return textInput.positionAt(pos);
    }

    /**
     * Returns the rectangle at the given position in the text.
     *
     * The x, y, and height properties correspond to the cursor that would
     * describe that position.
     *
     * @param int pos the text position
     * @param type:rectangle the cursor rectangle
     */
    function positionToRectangle(pos) {
        return textInput.positionToRectangle(pos);
    }

    // Set active focus to it's internal textInput.
    // Overriding QtQuick.Item forceActiveFocus function.
    function forceActiveFocus() {
        textInput.forceActiveFocus();
    }

    // Overriding QtQuick.Item activeFocus property.
    property alias activeFocus: textInput.activeFocus

    // TODO: fix default size
    implicitWidth: theme.defaultFont.mSize.width*12 + base.internalPadding*2
    implicitHeight: theme.defaultFont.mSize.height + base.internalPadding*2
    // TODO: needs to define if there will be specific graphics for
    //     disabled text fields
    opacity: enabled ? 1.0 : 0.5

    Private.TextFieldFocus {
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
        property real internalPadding: theme.defaultFont.mSize.height*0.3
    }

    MouseArea {
        id: mouseWatcher
        anchors.fill: hover
        hoverEnabled: true
        onClicked: {
            // If we don't set focus on click here then clicking between the
            // line of text and the bottom or top of the widget will not focus
            // it.
            textInput.forceActiveFocus();
        }
    }

    Text {
        anchors {
            left: parent.left
            right: parent.right
            verticalCenter: parent.verticalCenter
            leftMargin: base.margins.left + base.internalPadding
            rightMargin: base.margins.right + base.internalPadding
        }
        text: placeholderText
        visible: textInput.text == "" && !textField.activeFocus
        // XXX: using textColor and low opacity for theming placeholderText
        color: theme.buttonTextColor
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
            leftMargin: base.margins.left + base.internalPadding
            rightMargin: base.margins.right + (clearButton.opacity > 0 ? clearButton.width : 0)  + base.internalPadding
        }
        passwordCharacter: "•"
        selectByMouse: true
        color: theme.buttonTextColor
        enabled: textField.enabled
        clip: true
        focus: true
        onActiveFocusChanged: {
            if (!textField.activeFocus) {
                textInput.closeSoftwareInputPanel()
            }
        }
        onAccepted: textField.accepted()
        Keys.forwardTo: textField
    }

    PlasmaCore.IconItem {
        id: clearButton
        source: "edit-clear-locationbar-rtl"
        height: Math.max(textInput.height, theme.smallIconSize)
        width: height
        opacity: (textInput.text != "" && clearButtonShown) ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: 250
                easing.type: Easing.InOutQuad
            }
        }
        anchors {
            right: parent.right
            rightMargin: y
            verticalCenter: textInput.verticalCenter
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                textInput.text = ""
                textInput.forceActiveFocus()
            }
        }
    }
}
