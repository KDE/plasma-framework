/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2015 Marco Martin <notmart@gmail.com>
 * SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
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
    title: "Plasma Components 2 Button"
    contentItem: GridLayout {
        rowSpacing: PlasmaCore.Units.smallSpacing
        columnSpacing: PlasmaCore.Units.largeSpacing
        columns: 2

        PlasmaComponents.Label {
            text: "icon + text"
        }

        PlasmaComponents.Button {
            iconSource: "start-here-kde-plasma"
            text: "test"
        }

        PlasmaComponents.Label {
            text: "icon alone, should look small and square"
        }

        PlasmaComponents.Button {
            iconSource: "start-here-kde-plasma"
        }

        PlasmaComponents.Label {
            text: "text alone, should be about as wide as the text itself"
        }

        PlasmaComponents.Button {
            text: "test"
        }

        PlasmaComponents.Label {
            text: "This should look highlighted on load"
        }

        PlasmaComponents.Button {
            text: "test"
            focus: true
        }


        PlasmaComponents.Label {
            text: "long text, should expand to fit"
        }

        PlasmaComponents.Button {
            iconSource: "start-here-kde-plasma"
            text: "This is a really really really really long button"
        }

        PlasmaComponents.Label {
            text: "long text but constrained, should be 150px and elided"
        }

        PlasmaComponents.Button {
            iconSource: "start-here-kde-plasma"
            text: "This is a really really really really long button"
            Layout.maximumWidth: 150
        }

        PlasmaComponents.Label {
            text: "button with menu"
        }

        PlasmaComponents.Button {
            text: "test"
            menu: Menu {
                MenuItem {
                        text: "Cut"
                        shortcut: "Ctrl+X"
                    }

                    MenuItem {
                        text: "Copy"
                        shortcut: "Ctrl+C"
                    }

                    MenuItem {
                        text: "Paste"
                        shortcut: "Ctrl+V"
                    }

                    MenuSeparator { }

                    Menu {
                        title: "More Stuff"

                        MenuItem {
                            text: "Do Nothing"
                        }
                    }
            }
        }

        PlasmaComponents.Label {
            text: "button (with or without icon) and textfield should have the same height"
        }

        RowLayout {
            PlasmaComponents.Button {
                text: "test"
            }
            PlasmaComponents.Button {
                iconSource: "application-menu"
                text: "test"
            }
            PlasmaComponents.TextField {
            }
        }

        PlasmaComponents.Label {
            text: "minimum width property. Should be two letters wide"
        }

        RowLayout {
            PlasmaComponents.Button {
                text: "AA"
                implicitWidth: minimumWidth
            }
        }

    }
}

