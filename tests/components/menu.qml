import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents

Rectangle {
    width: 600
    height: 250
    color: "white"

    Flow {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        PlasmaComponents.Button {
            text: "Simple menu"
            onClicked: simpleMenu.open(0, height)

            PlasmaComponents.Menu {
                id: simpleMenu

                PlasmaComponents.MenuItem { text: "Hello" }
                PlasmaComponents.MenuItem { text: "This is just a simple" }
                PlasmaComponents.MenuItem { text: "Menu" }
                PlasmaComponents.MenuItem { text: "without separators" }
                PlasmaComponents.MenuItem { text: "and other stuff" }
            }
        }

        PlasmaComponents.Button {
            text: "Checkable menu items"
            onClicked: checkableMenu.open(0, height)

            PlasmaComponents.Menu {
                id: checkableMenu

                PlasmaComponents.MenuItem { text: "Apple"; checkable: true }
                PlasmaComponents.MenuItem { text: "Banana"; checkable: true }
                PlasmaComponents.MenuItem { text: "Orange"; checkable: true }
            }
        }


        PlasmaComponents.Button {
            text: "Icons"
            onClicked: iconsMenu.open(0, height)

            PlasmaComponents.Menu {
                id: iconsMenu

                PlasmaComponents.MenuItem { text: "Error"; icon: "dialog-error" }
                PlasmaComponents.MenuItem { text: "Warning"; icon: "dialog-warning" }
                PlasmaComponents.MenuItem { text: "Information"; icon: "dialog-information" }
            }
        }

        PlasmaComponents.Button {
            text: "Separators and sections"
            onClicked: sectionsMenu.open(0, height)

            PlasmaComponents.Menu {
                id: sectionsMenu

                PlasmaComponents.MenuItem { text: "A menu"; section: true }
                PlasmaComponents.MenuItem { text: "One entry" }
                PlasmaComponents.MenuItem { text: "Another entry" }
                PlasmaComponents.MenuItem { separator: true }
                PlasmaComponents.MenuItem { text: "One item" }
                PlasmaComponents.MenuItem { text: "Another item" }
            }
        }

        Row {
            spacing: units.smallSpacing

            PlasmaComponents.Button {
                id: minMaxButton
                text: "Fixed minimum and maximum width"
                onClicked: minMaxMenu.open(0, height)

                PlasmaComponents.Menu {
                    id: minMaxMenu

                    minimumWidth: minMaxButton.width
                    maximumWidth: limitMenuMaxWidth.checked ? minMaxButton.width : undefined // has a RESET property

                    PlasmaComponents.MenuItem { text: "Hello" }
                    PlasmaComponents.MenuItem { text: "This is just a simple" }
                    PlasmaComponents.MenuItem { text: "Menu" }
                    PlasmaComponents.MenuItem { text: "with some very very long text in one item that will "
                                                    + "make the menu super huge if you don't do anything about it" }
                    PlasmaComponents.MenuItem { text: "and other stuff" }
                }
            }

            PlasmaComponents.CheckBox {
                id: limitMenuMaxWidth
                anchors.verticalCenter: parent.verticalCenter
                text: "Limit maximum width"
                checked: true
            }
        }

        PlasmaComponents.Button {
            text: "Don't crash on null MenuItem action"
            onClicked: noActionCrashMenu.open(0, height)

            PlasmaComponents.Menu {
                id: noActionCrashMenu

                PlasmaComponents.MenuItem { text: "This is an item" }
                PlasmaComponents.MenuItem { text: "Below me should NOT be an empty item"}
                PlasmaComponents.MenuItem { action: null }
                PlasmaComponents.MenuItem { text: "I am not empty" }
            }
        }
    }
}
