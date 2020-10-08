/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 2 ToolButton"

    contentItem: Flow {
        spacing: PlasmaCore.Units.gridUnit

        PlasmaComponents.ToolButton {
            iconSource: "start-here-kde-plasma"
            text: "test"
            flat: true
        }
        PlasmaComponents.ToolButton {
            iconSource: "start-here-kde-plasma"
            flat: true
        }
        PlasmaComponents.ToolButton {
            text: "test"
            flat: true
        }
        PlasmaComponents.ToolButton {
            iconSource: "start-here-kde-plasma"
            text: "test"
            flat: false
        }
        PlasmaComponents.ToolButton {
            iconSource: "start-here-kde-plasma"
            flat: false
        }
        PlasmaComponents.ToolButton {
            text: "test"
            flat: false
        }
    }
}

