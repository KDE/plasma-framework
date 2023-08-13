/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami 2 as Kirigami
import org.kde.plasma.plasmoid
import org.kde.plasma.private.%{APPNAMELC} 1.0

WallpaperItem {
    id: root

    Rectangle {
        anchors.fill: parent
        color: Kirigami.Theme.backgroundColor
    }

    ColumnLayout {
        anchors.centerIn: parent

        Kirigami.Heading {
            Layout.alignment: Qt.AlignCenter
            level: 1
            text: wallpaper.configuration.DisplayText ||
                  i18n("<Please configure a text to display>")
        }

        PlasmaComponents.Label {
            Layout.alignment: Qt.AlignCenter
            text: HelloWorld.text
        }
    }
}
