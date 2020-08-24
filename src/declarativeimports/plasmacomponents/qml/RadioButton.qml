/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
