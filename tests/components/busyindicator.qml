/*
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 2 BusyIndicator"
    contentItem: ColumnLayout {
        spacing: PlasmaCore.Units.gridUnit

        PlasmaComponents.Label {
            wrapMode: Text.WordWrap
            text: "The BusyIndicator should have a height of 16px and should have a 1:1 aspect ratio"
            Layout.preferredWidth: Math.max(busyIndicatorLayout.width, root.implicitHeaderWidth)
        }
        PlasmaComponents.BusyIndicator {
            Layout.preferredHeight: 16
        }

        PlasmaComponents.Label {
            wrapMode: Text.WordWrap
            text: "The BusyIndicator should use its implicit size."
            Layout.preferredWidth: Math.max(busyIndicatorLayout.width, root.implicitHeaderWidth)
        }
        PlasmaComponents.BusyIndicator {}

        PlasmaComponents.Label {
            wrapMode: Text.WordWrap
            text: "The BusyIndicator should maintain a 1:1 aspect ratio, pause when unchecked and resume when checked."
        }

        RowLayout {
            spacing: PlasmaCore.Units.gridUnit

            PlasmaComponents.BusyIndicator {
                Layout.fillWidth: true
                Layout.fillHeight: true
                running: runningButton.checked
            }

            PlasmaComponents.CheckBox {
                id: runningButton
                text: "Running"
                checked: true
            }
        }
    }
}
