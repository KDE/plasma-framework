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
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Controls 1.2 as QtControls
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

/**
 * A plasma theme based toolbutton.
 *
 * ToolButtons which have their @c flat property set to false, which are not part
 * of a ButtonGroup, and have an empty text will become round.
 *
 * @inherit QtQuick.Controls.ToolButton
 */
QtControls.ToolButton {
    id: button

    /**
     * type:font
     *
     * This property holds the font used by the button label.
     *
     * See also Qt documentation for font type.
     */
    property font font: theme.defaultFont

    /*
     * overrides iconsource for compatibility
     */
    property alias iconSource: button.iconName

    /**
     * Smallest width this button can be to show all the contents.
     * Compatibility with old Button control.
     * The plasma style will update this property
     */
    property real minimumWidth: 0

    /**
     * Smallest height this button can be to show all the contents
     * Compatibility with old Button control.
     * The plasma style will update this property
     */
    property real minimumHeight: 0

    property bool flat: true

    style: Styles.ToolButtonStyle {}
}

