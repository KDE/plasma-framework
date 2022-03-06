/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root
    width: 100
    height: 100
    Layout.minimumWidth: PlasmaCore.Units.gridUnit * 20
    Layout.minimumHeight: column.implicitHeight


    ColumnLayout {
        id: column
        anchors.centerIn: parent
        PlasmaComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("String test")
        }
        PlasmaComponents.TextField {
            text: Plasmoid.configuration.Test
            onTextChanged: Plasmoid.configuration.Test = text
        }
        PlasmaComponents.CheckBox {
            enabled: true
            checked: Plasmoid.configuration.BoolTest
            text: i18n("Bool from config")
            onCheckedChanged: Plasmoid.configuration.BoolTest = checked
        }
        PlasmaComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("String from another group")
        }
        PlasmaComponents.TextField {
            text: Plasmoid.configuration.OtherTest
            onTextChanged: Plasmoid.configuration.OtherTest = text
        }
        PlasmaComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("Enum: displayed as int,\n written as string")
        }
        PlasmaComponents.TextField {
            text: Plasmoid.configuration.EnumTest
            onTextChanged: Plasmoid.configuration.EnumTest = text
        }
        PlasmaComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("Integer: minimum -1,\n maximum 100")
        }
        PlasmaComponents.TextField {
            text: Plasmoid.configuration.IntTest
            onTextChanged: Plasmoid.configuration.IntTest = text
        }
    }

}
