/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick.Controls 1.1
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

/**
 * This is a combobox which uses the plasma theme.
 *
 * Properties are the same as the QtQuick Controls ComboBox component
 *
 * @inherit QtQuick.Controls.ComboBox
 */
ComboBox {
    activeFocusOnPress: true

    style: Styles.ComboBoxStyle{}

}
