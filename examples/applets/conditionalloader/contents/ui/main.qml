/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0
import QtQuick.Layouts 1.0

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Rectangle {
    id: root
    color: "transparent"
    width: 100
    height: 100
    radius: 10
    smooth: true
    Layout.minimumWidth: PlasmaCore.Units.gridUnit * 20
    Layout.minimumHeight: column.implicitHeight

    Plasmoid.compactRepresentation: Component {
        Rectangle {
            MouseArea {
                anchors.fill: parent
                onClicked: Plasmoid.expanded = !Plasmoid.expanded
            }
        }
    }

    PlasmaExtras.ConditionalLoader {
        anchors.fill: parent
        when: Plasmoid.expanded
        source: Component {
            Item {
                ColumnLayout {
                    id: column
                    anchors.centerIn: parent
                    PlasmaComponents.Label {
                        anchors.horizontalCenter: parent.horizontalCenter
                        text: i18n("I'm an applet")
                    }
                    PlasmaComponents.Button {  
                        text: i18n("Background")
                        checked: Plasmoid.backgroundHints == 1
                        onClicked: {
                            print("Background hints: " + Plasmoid.backgroundHints)
                            if (Plasmoid.backgroundHints == 0) {
                                Plasmoid.backgroundHints = 1//TODO: make work "StandardBackground"
                                root.color = "transparent"
                            } else {
                                Plasmoid.backgroundHints = 0//TODO: make work "NoBackground"
                                root.color = "darkgreen"
                            }
                        }
                    }
                    PlasmaComponents.Button {
                        id: busyButton
                        text: i18n("Busy")
                        checked: Plasmoid.busy
                        onClicked: {
                            Plasmoid.busy = !Plasmoid.busy
                        }
                    }
                    PlasmaComponents.TextField {
                        implicitWidth: busyButton.width
                        text: Plasmoid.configuration.Test
                        onTextChanged: Plasmoid.configuration.Test = text
                    }
                    Component.onCompleted: {
                        print("Conditional component of test applet loaded")
                    }
                }
            }
        }
    }
    Component.onCompleted: {
        print("Test Applet loaded")
    }
}
