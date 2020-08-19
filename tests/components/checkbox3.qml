/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 3 CheckBox"
    contentItem: GridLayout {
        columnSpacing: PlasmaCore.Units.largeSpacing
        rowSpacing: PlasmaCore.Units.smallSpacing
        columns: 2

        PlasmaComponents.Label {
            text: "text"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
        }

        PlasmaComponents.Label {
            text: "icon"
        }
        PlasmaComponents.CheckBox {
            icon.name: "start-here-kde-plasma"
        }

        PlasmaComponents.Label {
            text: "text plus icon"
        }
        PlasmaComponents.CheckBox {
            text: "text"
            icon.name: "start-here-kde-plasma"
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
            checkState: Qt.Checked
        }

        PlasmaComponents.Label {
            text: "tri-state"
        }
        PlasmaComponents.CheckBox {
            text: "Some awesome checkbox"
            checkState: Qt.PartiallyChecked
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
            checkState: Qt.Checked
        }
    }
}

