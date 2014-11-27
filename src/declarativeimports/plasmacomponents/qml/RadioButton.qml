/*
 *   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
 *   Copyright 2014 Marco Martin <mart@kde.org>
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
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

/**
 * A radio button component consists of a radio button and a line of text. Only
 * one item in a list may be selected at a time. Once an item is selected, it
 * can be deselected only by selecting another item. Initial item selection may
 * be set at the list creation. If not set, the list is shown without a
 * selection.
 *
 * When a check box has the focus, its state can be toggled using the
 * Qt.Key_Select, Qt.Key_Return, and Qt.Key_Enter hardware keys that send the
 * clicked signal.
 *
 * @inherit QtQuick.Controls.RadioButton
 */
QtControls.RadioButton {
    id: radioButton

    style: Styles.RadioButtonStyle {}
}
