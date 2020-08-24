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
 * A check box is a component that can be switched on (checked) or off
 * (unchecked). Check boxes are typically used to represent features in an
 * application that can be enabled or disabled without affecting others, but
 * different types of behavior can be implemented. When a check box is checked
 * or unchecked it sends a clicked signal for the application to handle.
 *
 * When a check box has the focus, its state can be toggled using the
 * Qt.Key_Select, Qt.Key_Return, and Qt.Key_Enter hardware keys that send the
 * clicked signal.
 *
 * @inherit QtQuick.Controls.CheckBox
 */
QtControls.CheckBox {
    id: checkBox

    style: Styles.CheckBoxStyle {}
}
