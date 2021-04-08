/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick 2.1

import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

import org.kde.plasma.private.%{APPNAMELC} 1.0

Item {
    id: root

    Rectangle {
        anchors.fill: parent
        color: PlasmaCore.Theme.backgroundColor
    }

    ColumnLayout {
        anchors.centerIn: parent

        PlasmaExtras.Heading {
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
