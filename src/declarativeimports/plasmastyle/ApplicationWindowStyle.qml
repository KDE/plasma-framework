/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.3
import QtQuick.Controls.Styles 1.3 as QtQuickControlStyle
import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * \internal
 */
QtQuickControlStyle.ApplicationWindowStyle {

    property Component background: Rectangle {
        visible: !styleData.hasColor
        color: PlasmaCore.ColorScope.backgroundColor
    }
}
