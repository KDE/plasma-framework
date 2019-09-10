import QtQuick 2.0

import org.kde.plasma.components 3.0 as PlasmaComponents
import QtQuick.Layouts 1.2

Rectangle
{
    width: 500
    height: 500
    color: "white"

    Grid {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20
        columns: 2

        Label {
            text: "icon + text"
        }

        PlasmaComponents.Button {
            icon.name: "list-remove"
            text: "test"
        }

        Label {
            text: "icon alone, should look small and square"
        }

        PlasmaComponents.Button {
            icon.name: "list-remove"
        }

        Label {
            text: "text alone, should be about 12 chars wide"
        }

        PlasmaComponents.Button {
            text: "test"
        }


        Label {
            text: "long text, should expand to fit"
        }

        PlasmaComponents.Button {
            icon.name: "list-remove"
            text: "This is a really really really really long button"
        }

        Label {
            text: "long text but constrained, should be 150px and elided"
        }

        PlasmaComponents.Button {
            icon.name: "list-remove"
            text: "This is a really really really really long button"
            width: 150
        }


        Label {
            text: "button (with or without icon) and textfield should have the same height"
        }

        RowLayout {
            PlasmaComponents.Button {
                text: "test"
            }
            PlasmaComponents.Button {
                icon.name: "application-menu"
                text: "test"
            }
            PlasmaComponents.TextField {
            }
        }

        Label {
            text: "minimum width property. Should be two letters wide"
        }

        RowLayout {
            PlasmaComponents.Button {
                text: "AA"
//                 implicitWidth: minimumWidth FIXME, there is no equivalent?
            }
        }

    }
}

