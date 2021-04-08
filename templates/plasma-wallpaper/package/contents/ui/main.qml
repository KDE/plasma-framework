/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: PlasmaCore.Theme.backgroundColor
    }

    PlasmaExtras.Heading {
        anchors.centerIn: parent
        level: 1
        text: wallpaper.configuration.DisplayText ||
              i18n("<Please configure a text to display>")
    }
}
