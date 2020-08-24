/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Controls 1.2 as QtControls
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

import org.kde.plasma.core 2.0 as PlasmaCore

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
    property font font: PlasmaCore.Theme.defaultFont

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

