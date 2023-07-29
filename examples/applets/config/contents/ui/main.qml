/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2 as Kirigami

Item {
    id: root
    width: 100
    height: 100
    Layout.minimumWidth: Kirigami.Units.gridUnit * 20
    Layout.minimumHeight: column.implicitHeight


    ColumnLayout {
        id: column
        anchors.centerIn: parent
        PC3.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("String test")
        }
        PC3.TextField {
            text: Plasmoid.configuration.Test
            onTextChanged: Plasmoid.configuration.Test = text
        }
        PC3.CheckBox {
            enabled: true
            checked: Plasmoid.configuration.BoolTest
            text: i18n("Bool from config")
            onCheckedChanged: Plasmoid.configuration.BoolTest = checked
        }
        PC3.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("String from another group")
        }
        PC3.TextField {
            text: Plasmoid.configuration.OtherTest
            onTextChanged: Plasmoid.configuration.OtherTest = text
        }
        PC3.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("Enum: displayed as int,\n written as string")
        }
        PC3.TextField {
            text: Plasmoid.configuration.EnumTest
            onTextChanged: Plasmoid.configuration.EnumTest = text
        }
        PC3.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("Integer: minimum -1,\n maximum 100")
        }
        PC3.TextField {
            text: Plasmoid.configuration.IntTest
            onTextChanged: Plasmoid.configuration.IntTest = text
        }
    }

}
