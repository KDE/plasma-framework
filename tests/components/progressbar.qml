/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 2 ProgressBar"
    property int orientation: orientationCombo.model[orientationCombo.currentIndex].value
    property int progressBarWidth: testProgressBar.width

    PlasmaComponents.ProgressBar {
        id: testProgressBar
        visible: false
    }

    contentItem: GridLayout {
        columns: 6
        columnSpacing: PlasmaCore.Units.largeSpacing
        rowSpacing: PlasmaCore.Units.largeSpacing

        ColumnLayout {
            PlasmaComponents.Label {
                text: "0%"
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 100
                value: 0
                orientation: root.orientation
            }
        }

        ColumnLayout {
            PlasmaComponents.Label {
                text: "50%"
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 100
                value: 50
                orientation: root.orientation
            }
        }

        ColumnLayout {
            PlasmaComponents.Label {
                text: "100%"
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 100
                value: 100
                orientation: root.orientation
            }
        }

        ColumnLayout {
            PlasmaComponents.Label {
                id: progressBarAndSliderLabel
                text: "The progress bar and slider grooves should have the same visual width."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            GridLayout {
                id: progressBarAndSliderGrid
                columns: orientation === Qt.Vertical ? 2 : 1
                rows: orientation === Qt.Vertical ? 1 : 2
                PlasmaComponents.ProgressBar {
                    id: progressBar
                    minimumValue: 0
                    maximumValue: 100
                    value: 50
                    orientation: root.orientation
                }
                PlasmaComponents.Slider {
                    width: progressBar.width
                    height: progressBar.height
                    minimumValue: 0
                    maximumValue: 100
                    value: 50
                    orientation: root.orientation
                }
            }
        }

        ColumnLayout {
            PlasmaComponents.Label {
                text: "Min: 0; Max: 200; Value: 1\nMake sure the bar does not leak outside."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 200
                value: 1
                orientation: root.orientation
            }
        }

        ColumnLayout {
            PlasmaComponents.Label {
                text: "Min: 0; Max: 100; Value: 110\nThe progress bar should look like it is at 100%."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 100
                value: 110
                orientation: root.orientation
            }
        }

        ColumnLayout {
            PlasmaComponents.Label {
                text: "Min: -100; Max: 100; Value: 0\nThe progress bar should look like it is at 50%."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            PlasmaComponents.ProgressBar {
                minimumValue: -100
                maximumValue: 100
                value: 0
                orientation: root.orientation
            }
        }

        ColumnLayout {
            PlasmaComponents.Label {
                text: "Min: 0; Max: 100; Value: -10\nThe progress bar should look like it is at 0%."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            PlasmaComponents.ProgressBar {
                minimumValue: 0
                maximumValue: 100
                value: -10
                orientation: root.orientation
            }
        }

        ColumnLayout {
            PlasmaComponents.Label {
                text: "This should have a continuous movement from one end to the other and back."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            PlasmaComponents.ProgressBar {
                indeterminate: indeterminateCheckBox.checked
                value: 0.5
                orientation: root.orientation
            }
        }

        ColumnLayout {
            PlasmaComponents.Label {
                text: "Checking and unchecking should not break the layout. The progress bar should look like it is at 50% if unchecked."
                wrapMode: Text.WordWrap
                Layout.preferredWidth: progressBarWidth
            }
            PlasmaComponents.CheckBox {
                id: indeterminateCheckBox
                text: "Indeterminate"
                checked: true
            }
        }

        ColumnLayout {
            PlasmaComponents.Label {
                text: "Slider orientation"
            }
            PlasmaComponents.ComboBox {
                id: orientationCombo
                model: [
                    {text: "Horizontal", value: Qt.Horizontal},
                    {text: "Vertical", value: Qt.Vertical}
                ]
            }
        }
    }
}
