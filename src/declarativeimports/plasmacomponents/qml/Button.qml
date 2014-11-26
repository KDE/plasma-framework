/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2011 by Mark Gaiser <markg85@gmail.com>
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


/**
 * A button with optional label and icon which uses the plasma theme.
 *
 * This button component can also be used as a checkable button by using
 * the checkable and checked properties for that.  Plasma theme is the
 * theme which changes via the systemsetting-workspace appearance -desktop
 * theme.
 */
import QtQuick 2.1
import QtQuick.Controls 1.2 as QtControls
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

/**
 * The push button is perhaps the most commonly used widget in any graphical user interface.
 * Pushing (or clicking) a button commands the computer to perform some action
 * or answer a question. Common examples of buttons are OK, Apply, Cancel,
 * Close, Yes, No, and Help buttons.
 *
 * @inherit QtQuick.Controls.Button
 */
QtControls.Button {
    id: root
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
    property alias iconSource: root.iconName

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

    style: Styles.ButtonStyle {}
}
