import QtQuick 2.0
import QtQuick.Layouts 1.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "PlasmaComponents 3 Button"
    contentItem: GridLayout {
        rowSpacing: PlasmaCore.Units.smallSpacing
        columnSpacing: PlasmaCore.Units.largeSpacing
        columns: 2

        PlasmaComponents.Label {
            text: "icon + text"
        }

        PlasmaComponents.Button {
            icon.name: "start-here-kde-plasma"
            text: "test"
        }

        PlasmaComponents.Label {
            text: "icon alone, should look small and square"
        }

        PlasmaComponents.Button {
            icon.name: "start-here-kde-plasma"
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
            icon.name: "start-here-kde-plasma"
            text: "This is a really really really really long button"
        }

        PlasmaComponents.Label {
            text: "long text but constrained, should be 150px and elided"
        }

        PlasmaComponents.Button {
            icon.name: "start-here-kde-plasma"
            text: "This is a really really really really long button"
            Layout.maximumWidth: 150
        }


        PlasmaComponents.Label {
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

        PlasmaComponents.Label {
            text: "minimum width property. Should be two letters wide"
        }

        RowLayout {
            PlasmaComponents.Button {
                text: "AA"
                implicitWidth: Layout.minimumWidth
            }
            PlasmaComponents.Button {
                icon.name: "application-menu"
                text: "AA"
                implicitWidth: Layout.minimumWidth
            }
            PlasmaComponents.Button {
                icon.name: "application-menu"
                implicitWidth: Layout.minimumWidth
            }
        }

        PlasmaComponents.Label {
            text: "Display property"
        }
        RowLayout {
            PlasmaComponents.Button {
                icon.name: "application-menu"
                text: "Icon Only"
                display: PlasmaComponents.Button.IconOnly
            }
            PlasmaComponents.Button {
                icon.name: "application-menu"
                text: "Text Beside Icon"
                display: PlasmaComponents.Button.TextBesideIcon
            }
            PlasmaComponents.Button {
                icon.name: "application-menu"
                text: "Text Under Icon"
                display: PlasmaComponents.Button.TextUnderIcon
            }
            PlasmaComponents.Button {
                icon.name: "application-menu"
                text: "Text Only"
                display: PlasmaComponents.Button.TextOnly
            }
        }

    }
}

