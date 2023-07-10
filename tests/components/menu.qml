/*
 * SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>
 * SPDX-FileCopyrightText: 2019 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.20 as Kirigami

ComponentBase {
    id: root
    title: "Plasma Extras Menu"
    contentItem: ColumnLayout {
        spacing: Kirigami.Units.gridUnit

        PC3.Button {
            text: "Simple menu"
            onClicked: simpleMenu.open(0, height)

            PlasmaExtras.Menu {
                id: simpleMenu

                PlasmaExtras.MenuItem { text: "Hello" }
                PlasmaExtras.MenuItem { text: "This is just a simple" }
                PlasmaExtras.MenuItem { text: "Menu" }
                PlasmaExtras.MenuItem { text: "without separators" }
                PlasmaExtras.MenuItem { text: "and other stuff" }
            }
        }

        PC3.Button {
            text: "Checkable menu items"
            onClicked: checkableMenu.open(0, height)

            PlasmaExtras.Menu {
                id: checkableMenu

                PlasmaExtras.MenuItem { text: "Apple"; checkable: true }
                PlasmaExtras.MenuItem { text: "Banana"; checkable: true }
                PlasmaExtras.MenuItem { text: "Orange"; checkable: true }
            }
        }


        PC3.Button {
            text: "Icons"
            onClicked: iconsMenu.open(0, height)

            PlasmaExtras.Menu {
                id: iconsMenu

                PlasmaExtras.MenuItem { text: "Error"; icon: "dialog-error" }
                PlasmaExtras.MenuItem { text: "Warning"; icon: "dialog-warning" }
                PlasmaExtras.MenuItem { text: "Information"; icon: "dialog-information" }
            }
        }

        PC3.Button {
            text: "Separators and sections"
            onClicked: sectionsMenu.open(0, height)

            PlasmaExtras.Menu {
                id: sectionsMenu

                PlasmaExtras.MenuItem { text: "A menu"; section: true }
                PlasmaExtras.MenuItem { text: "One entry" }
                PlasmaExtras.MenuItem { text: "Another entry" }
                PlasmaExtras.MenuItem { separator: true }
                PlasmaExtras.MenuItem { text: "One item" }
                PlasmaExtras.MenuItem { text: "Another item" }
            }
        }

        RowLayout {
            spacing: Kirigami.Units.smallSpacing

            PC3.Button {
                id: minMaxButton
                text: "Fixed minimum and maximum width"
                onClicked: minMaxMenu.open(0, height)

                PlasmaExtras.Menu {
                    id: minMaxMenu

                    minimumWidth: minMaxButton.width
                    maximumWidth: limitMenuMaxWidth.checked ? minMaxButton.width : undefined // has a RESET property

                    PlasmaExtras.MenuItem { text: "Hello" }
                    PlasmaExtras.MenuItem { text: "This is just a simple" }
                    PlasmaExtras.MenuItem { text: "Menu" }
                    PlasmaExtras.MenuItem { text: "with some very very long text in one item that will "
                                                    + "make the menu super huge if you don't do anything about it" }
                    PlasmaExtras.MenuItem { text: "and other stuff" }
                }
            }

            PC3.CheckBox {
                id: limitMenuMaxWidth
                anchors.verticalCenter: parent.verticalCenter
                text: "Limit maximum width"
                checked: true
            }
        }

        PC3.Button {
            text: "Don't crash on null MenuItem action"
            onClicked: noActionCrashMenu.open(0, height)

            PlasmaExtras.Menu {
                id: noActionCrashMenu

                PlasmaExtras.MenuItem { text: "This is an item" }
                PlasmaExtras.MenuItem { text: "Below me should NOT be an empty item"}
                PlasmaExtras.MenuItem { action: null }
                PlasmaExtras.MenuItem { text: "I am not empty" }
            }
        }
    }
}
