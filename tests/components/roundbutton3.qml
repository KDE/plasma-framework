/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

ComponentBase {
    id: root
    title: "Plasma Components 3 RoundButton"
    contentItem: ColumnLayout {
        GridLayout {
            rowSpacing: PlasmaCore.Units.smallSpacing
            columnSpacing: PlasmaCore.Units.largeSpacing
            columns: 2

            PlasmaComponents.Label {
                text: "icon + text"
            }

            PlasmaComponents.RoundButton {
                icon.name: "start-here-kde-plasma"
                text: "test"
            }

            PlasmaComponents.Label {
                text: "icon alone, should look small and square"
            }

            PlasmaComponents.RoundButton {
                icon.name: "start-here-kde-plasma"
            }

            PlasmaComponents.Label {
                text: "text alone, should be about as wide as the text itself"
            }

            PlasmaComponents.RoundButton {
                text: "test"
            }

            PlasmaComponents.Label {
                text: "This should look highlighted on load"
            }

            PlasmaComponents.RoundButton {
                text: "test"
                focus: true
            }

            PlasmaComponents.Label {
                text: "long text, should expand to fit"
            }

            PlasmaComponents.RoundButton {
                icon.name: "start-here-kde-plasma"
                text: "This is a really really really really long button"
            }

            PlasmaComponents.Label {
                text: "long text but constrained, should be 150px and elided"
            }

            PlasmaComponents.RoundButton {
                icon.name: "start-here-kde-plasma"
                text: "This is a really really really really long button"
                Layout.maximumWidth: 150
            }


            PlasmaComponents.Label {
                text: "button (with or without icon) and textfield should have the same height"
            }

            RowLayout {
                PlasmaComponents.RoundButton {
                    text: "test"
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "test"
                }
                PlasmaComponents.TextField {
                }
            }

            PlasmaComponents.Label {
                text: "minimum width property. Should be two letters wide"
            }

            RowLayout {
                PlasmaComponents.RoundButton {
                    text: "AA"
                    implicitWidth: Layout.minimumWidth
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "AA"
                    implicitWidth: Layout.minimumWidth
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    implicitWidth: Layout.minimumWidth
                }
            }

            PlasmaComponents.Label {
                text: "Display property"
            }
            RowLayout {
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Icon Only"
                    display: PlasmaComponents.RoundButton.IconOnly
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Beside Icon"
                    display: PlasmaComponents.RoundButton.TextBesideIcon
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Under Icon"
                    display: PlasmaComponents.RoundButton.TextUnderIcon
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Only"
                    display: PlasmaComponents.RoundButton.TextOnly
                }
            }

        }
        PlasmaExtras.Heading {
            text: "Flat Buttons"
        }
        GridLayout {
            rowSpacing: PlasmaCore.Units.smallSpacing
            columnSpacing: PlasmaCore.Units.largeSpacing
            columns: 2

            PlasmaComponents.Label {
                text: "icon + text"
            }

            PlasmaComponents.RoundButton {
                icon.name: "start-here-kde-plasma"
                text: "test"
                flat: true
            }

            PlasmaComponents.Label {
                text: "icon alone, should look small and square"
            }

            PlasmaComponents.RoundButton {
                icon.name: "start-here-kde-plasma"
                flat: true
            }

            PlasmaComponents.Label {
                text: "text alone, should be about as wide as the text itself"
            }

            PlasmaComponents.RoundButton {
                text: "test"
                flat: true
            }

            PlasmaComponents.Label {
                text: "long text, should expand to fit"
            }

            PlasmaComponents.RoundButton {
                icon.name: "start-here-kde-plasma"
                text: "This is a really really really really long button"
                flat: true
            }

            PlasmaComponents.Label {
                text: "long text but constrained, should be 150px and elided"
            }

            PlasmaComponents.RoundButton {
                icon.name: "start-here-kde-plasma"
                text: "This is a really really really really long button"
                Layout.maximumWidth: 150
                flat: true
            }


            PlasmaComponents.Label {
                text: "button (with or without icon) and textfield should have the same height"
            }

            RowLayout {
                PlasmaComponents.RoundButton {
                    text: "test"
                    flat: true
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "test"
                    flat: true
                }
                PlasmaComponents.TextField {
                }
            }

            PlasmaComponents.Label {
                text: "minimum width property. Should be two letters wide"
            }

            RowLayout {
                PlasmaComponents.RoundButton {
                    text: "AA"
                    implicitWidth: Layout.minimumWidth
                    flat: true
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "AA"
                    implicitWidth: Layout.minimumWidth
                    flat: true
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    implicitWidth: Layout.minimumWidth
                    flat: true
                }
            }

            PlasmaComponents.Label {
                text: "Display property"
            }
            RowLayout {
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Icon Only"
                    display: PlasmaComponents.RoundButton.IconOnly
                    flat: true
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Beside Icon"
                    display: PlasmaComponents.RoundButton.TextBesideIcon
                    flat: true
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Under Icon"
                    display: PlasmaComponents.RoundButton.TextUnderIcon
                    flat: true
                }
                PlasmaComponents.RoundButton {
                    icon.name: "application-menu"
                    text: "Text Only"
                    display: PlasmaComponents.RoundButton.TextOnly
                    flat: true
                }
            }
        }
    }
}
