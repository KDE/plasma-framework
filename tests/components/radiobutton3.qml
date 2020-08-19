/*
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 George Vogiatzis <Gvgeo@protonmail.com>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 3 RadioButton"
    contentItem: GridLayout {
        columnSpacing: PlasmaCore.Units.largeSpacing
        rowSpacing: PlasmaCore.Units.smallSpacing
        columns: 2

        PlasmaComponents.Label {
            text: "text"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
        }

        PlasmaComponents.Label {
            text: "focus"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            focus: true
        }

        PlasmaComponents.Label {
            text: "checked"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            checked: true
        }

        PlasmaComponents.Label {
            text: "disabled"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            enabled: false
        }

        PlasmaComponents.Label {
            text: "disabled and checked"
        }
        PlasmaComponents.RadioButton {
            text: "Some awesome radiobutton"
            enabled: false
            checked: true
        }
    }
}

