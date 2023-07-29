/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick 2.1

import org.kde.kirigami 2 as Kirigami

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor
    }

    Kirigami.Heading {
        anchors.centerIn: parent
        level: 1
        text: wallpaper.configuration.DisplayText ||
              i18n("<Please configure a text to display>")
    }
}
