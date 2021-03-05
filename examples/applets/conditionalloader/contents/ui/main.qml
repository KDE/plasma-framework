/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0
import QtQuick.Layouts 1.0

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
                onClicked: plasmoid.expanded = !plasmoid.expanded
            }
        }
    }

    PlasmaExtras.ConditionalLoader {
        anchors.fill: parent
        when: plasmoid.expanded
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
                        checked: plasmoid.backgroundHints == 1
                        onClicked: {
                            print("Background hints: " + plasmoid.backgroundHints)
                            if (plasmoid.backgroundHints == 0) {
                                plasmoid.backgroundHints = 1//TODO: make work "StandardBackground"
                                root.color = "transparent"
                            } else {
                                plasmoid.backgroundHints = 0//TODO: make work "NoBackground"
                                root.color = "darkgreen"
                            }
                        }
                    }
                    PlasmaComponents.Button {
                        id: busyButton
                        text: i18n("Busy")
                        checked: plasmoid.busy
                        onClicked: {
                            plasmoid.busy = !plasmoid.busy
                        }
                    }
                    PlasmaComponents.TextField {
                        implicitWidth: busyButton.width
                        text: plasmoid.configuration.Test
                        onTextChanged: plasmoid.configuration.Test = text
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
