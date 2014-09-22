import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents
import QtQuick.Controls 1.2

Rectangle
{
    width: 500
    height: 300
    color: "white"

    Flow {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        PlasmaComponents.Button {
            iconSource: "list-remove"
            text: "test"
        }
        PlasmaComponents.Button {
            iconSource: "list-remove"
        }
        PlasmaComponents.Button {
            text: "test"
        }

        PlasmaComponents.Button{
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
    }
}

