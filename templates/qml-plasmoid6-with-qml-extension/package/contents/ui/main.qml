/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents

import org.kde.plasma.private.%{APPNAMELC} 1.0

Item {
    Plasmoid.fullRepresentation: ColumnLayout {
        anchors.fill: parent
        PlasmaComponents.Label {
            Layout.alignment: Qt.AlignCenter
            text: HelloWorld.text
        }
    }
}
