/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Controls 1.2 as QtControls
import org.kde.plasma.core 2.0 as PlasmaCore
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

/**
   @class TextArea
   @short A scrollable text area
   @inherit QtQuick.Controls.TextArea
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
    property bool interactive: !selectByMouse
    onInteractiveChanged: selectByMouse = !interactive

    /**
     * type:int
     * Maximum width that the text content can have.
     */
    property int contentMaxWidth: contentItem.width

    /**
     * type:int
     * Maximum height that the text content can have.
     */
    property int contentMaxHeight: contentItem.height

    // Overriding QtQuick.Item activeFocus property.
    //property alias activeFocus: textEdit.activeFocus
    implicitWidth: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width*25 + style.textMargin
    implicitHeight: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height*8 + style.textMargin

    Label {
        anchors.centerIn: parent
        width: Math.min(implicitWidth, parent.width)
        text: textArea.placeholderText
        visible: textArea.text == "" && !textArea.activeFocus
        color: PlasmaCore.Theme.viewTextColor
        horizontalAlignment: Text.AlignHCenter
        opacity: 0.5
    }
    
    style: Styles.TextAreaStyle {}
}
