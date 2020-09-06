/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2011 Mark Gaiser <markg85@gmail.com>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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

import org.kde.plasma.core 2.0 as PlasmaCore

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
    property font font: PlasmaCore.Theme.defaultFont

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
