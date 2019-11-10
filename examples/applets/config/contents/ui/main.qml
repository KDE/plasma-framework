/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.1
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root
    width: 100
    height: 100
    Layout.minimumWidth: units.gridUnit * 20
    Layout.minimumHeight: column.implicitHeight


    ColumnLayout {
        id: column
        anchors.centerIn: parent
        PlasmaComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("String test")
        }
        PlasmaComponents.TextField {
            text: plasmoid.configuration.Test
            onTextChanged: plasmoid.configuration.Test = text
        }
        PlasmaComponents.CheckBox {
            enabled: true
            checked: plasmoid.configuration.BoolTest
            text: i18n("Bool from config")
            onCheckedChanged: plasmoid.configuration.BoolTest = checked
        }
        PlasmaComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("String from another group")
        }
        PlasmaComponents.TextField {
            text: plasmoid.configuration.OtherTest
            onTextChanged: plasmoid.configuration.OtherTest = text
        }
        PlasmaComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("Enum: displayed as int,\n written as string")
        }
        PlasmaComponents.TextField {
            text: plasmoid.configuration.EnumTest
            onTextChanged: plasmoid.configuration.EnumTest = text
        }
        PlasmaComponents.Label {
            anchors.horizontalCenter: parent.horizontalCenter
            text: i18n("Integer: minimum -1,\n maximum 100")
        }
        PlasmaComponents.TextField {
            text: plasmoid.configuration.IntTest
            onTextChanged: plasmoid.configuration.IntTest = text
        }
    }

}
