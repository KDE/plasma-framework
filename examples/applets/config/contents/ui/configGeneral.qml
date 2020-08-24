/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 1.0 as QtControls

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents


Item {
    id: iconsPage
    width: childrenRect.width
    height: childrenRect.height
    implicitWidth: mainColumn.implicitWidth
    implicitHeight: pageColumn.implicitHeight

    property alias cfg_Test: testConfigField.text
    property alias cfg_BoolTest: testBoolConfigField.checked

    Column {
        id: pageColumn
        anchors.fill: parent
        spacing: 4
        Row {
            QtControls.Label {
                text: "Text Config value"
            }
            QtControls.TextField {
                id: testConfigField
            }
        }
        Row {
            QtControls.Label {
                text: "Bool Config value"
            }
            QtControls.CheckBox {
                id: testBoolConfigField
            }
        }
    }
}
