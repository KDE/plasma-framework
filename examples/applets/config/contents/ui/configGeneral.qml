/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls 2.15 as QQC2

import org.kde.plasma.core 2.0 as PlasmaCore

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
            QQC2.Label {
                text: "Text Config value"
            }
            QQC2.TextField {
                id: testConfigField
            }
        }
        Row {
            QQC2.Label {
                text: "Bool Config value"
            }
            QQC2.CheckBox {
                id: testBoolConfigField
            }
        }
    }
}
