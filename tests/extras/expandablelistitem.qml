import QtQuick 2.15
import QtQuick.Layouts 1.5

import QtQml.Models 2.15
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2 as Kirigami

Rectangle {
    height: 800
    width: 500
    color: Kirigami.Theme.backgroundColor
    PlasmaComponents.ScrollView {
        anchors.fill: parent
        ListView {
            anchors.fill: parent
            focus: true
            currentIndex: -1
            clip: true
            model: myModel
            highlight: PlasmaExtras.Highlight {}
            highlightMoveDuration: Kirigami.Units.longDuration
            highlightResizeDuration: Kirigami.Units.longDuration
            delegate: PlasmaExtras.ExpandableListItem {
                title: model.title
                subtitle: model.subtitle
                icon: model.icon
                isBusy: model.busy
                subtitleCanWrap: model.subtitleCanWrap || false

                customExpandedViewContent: Component {
                    ColumnLayout {
                        PlasmaComponents.Label {
                            text: "I am some expanded text"
                        }
                        PlasmaComponents.Button {
                            text: "with an expanded button"
                        }
                    }
                }
            }
        }
    }

    ListModel {
        id: myModel
        ListElement {
            title: "Item 1"
            subtitle: "Default with icon"
            icon: "system-file-manager"
            isDefault: true
        }
        ListElement {
            title: "Item 2"
            subtitle: "A really long subtitle that probably won't fit in this constrained example because of how long it is."
            isDefault: false
        }
        ListElement {
            title: "Item 4"
            subtitle: "Busy"
            isDefault: false
            busy: true
        }

    }
}
