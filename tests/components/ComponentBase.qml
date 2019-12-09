import QtQuick 2.0
import QtQuick.Layouts 1.2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

PlasmaCore.ColorScope
{
    id: root
    width: 500
    height: 300
    property bool invertedColors: false
    default property alias children: container.children
    colorGroup: invertedColors ? PlasmaCore.Theme.ComplementaryColorGroup : PlasmaCore.Theme.NormalColorGroup

    Rectangle {
        anchors.fill: parent
        color:  PlasmaCore.ColorScope.backgroundColor
    }
 
    PlasmaComponents.Label {
        id: label
        text: root.invertedColors ? "Invert" : "Normal"
        MouseArea {
            anchors.fill: parent
            onClicked: root.invertedColors = !root.invertedColors
        }
    } 
    
    Item {
        id: container
        anchors.top: label.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
    }
}
