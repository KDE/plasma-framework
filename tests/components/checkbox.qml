/*
 * SPDX-FileCopyrightText: 2017 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.2
import QtQuick.Controls 1.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 2 CheckBox"
    contentItem: GridLayout {
        rowSpacing: PlasmaCore.Units.smallSpacing
        columnSpacing: PlasmaCore.Units.largeSpacing
        columns: 2

        PlasmaComponents.Label {
            text: "text"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
        }

        PlasmaComponents.Label {
            text: "focus"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            focus: true
        }

        PlasmaComponents.Label {
            text: "checked"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            checkedState: Qt.Checked
        }

        PlasmaComponents.Label {
            text: "tri-state"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            checkedState: Qt.PartiallyChecked
        }

        PlasmaComponents.Label {
            text: "disabled"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            enabled: false
        }

        PlasmaComponents.Label {
            text: "disabled and checked"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            enabled: false
            checkedState: Qt.Checked
        }
    }
}

