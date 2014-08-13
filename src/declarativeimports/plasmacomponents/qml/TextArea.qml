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
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Controls 1.2 as QtControls
import org.kde.plasma.core 2.0 as PlasmaCore
import "styles" as Styles

/**
 * A plasma theme based text area.
 */
QtControls.TextArea {
    id: textArea

    /**
     * Whether the text field is highlighted or not.
     *
     * If it is true then the problematic lines will be highlighted.
     *
     * @warning unimplemented in plasma components.
     */
    property bool errorHighlight

    /**
     * The text displayed when the text property is empty.
     *
     * The default value is an empty string, meaning no placeholderText shown.
     */
    property string placeholderText


    // Plasma API
    /**
     * type:bool
     * This property describes whether the user can interact with the TextArea
     * flicking content. A user cannot drag or flick a TextArea that is not
     * interactive. This property is useful for temporarily disabling
     * flicking.
     */
    property bool interactive

    /**
     * type:int
     * Maximum width that the text content can have.
     */
    property int contentMaxWidth

    /**
     * type:int
     * Maximum height that the text content can have.
     */
    property int contentMaxHeight

    // Overriding QtQuick.Item activeFocus property.
    //property alias activeFocus: textEdit.activeFocus
    implicitWidth: theme.mSize(theme.defaultFont).width*25 + base.internalPadding*2
    implicitHeight: theme.mSize(theme.defaultFont).height*8 + base.internalPadding*2

    opacity: enabled ? 1.0 : 0.5

    Label {
        anchors.centerIn: parent
        width: Math.min(implicitWidth, parent.width)
        text: textArea.placeholderText
        visible: textArea.text == "" && !textArea.activeFocus
        color: theme.viewTextColor
        opacity: 0.5
    }
    
    style: Styles.TextAreaStyle {}
}
