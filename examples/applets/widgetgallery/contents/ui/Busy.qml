/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.Page {
    height: childrenRect.height
    property int implicitHeight: childrenRect.height

    tools: PlasmaComponents.ToolBarLayout {
        spacing: 5
        PlasmaComponents.ToolButton {
            visible: pageStack.depth > 1
            iconSource: "go-previous"
            onClicked: pageStack.pop()
        }
        PlasmaComponents.ProgressBar {
            value: 0.3
        }
        PlasmaComponents.TextField {
            clearButtonShown: true
            text: "Busy widgets"
        }
    }

    Flickable {
        id: flickable
        contentWidth: column.width
        contentHeight: column.height
        clip: true
        anchors.fill: parent

        Item {
            width: Math.max(flickable.width, column.width)
            height: column.height
            Column {
                id: column
                spacing: 20
                anchors.horizontalCenter: parent.horizontalCenter

                PlasmaComponents.Label {
                    font.pixelSize: 20
                    text: "Busy Indicator"
                }

                PlasmaComponents.BusyIndicator { }

                PlasmaComponents.BusyIndicator { running: true }

                PlasmaComponents.Label {
                    font.pixelSize: 20
                    text: "Progress Bar"
                }

                PlasmaComponents.Label { text: "Horizontal" }

                PlasmaComponents.ProgressBar {
                    value: 0.3
                }

                PlasmaComponents.ProgressBar {
                    indeterminate: true
                }

                PlasmaComponents.ProgressBar {
                    minimumValue: 0
                    maximumValue: 100
                    value: 30
                }

                PlasmaComponents.Label { text: "Vertical" }
                Row {
                    spacing: 20
                    PlasmaComponents.ProgressBar {
                        value: 0.3
                        orientation: Qt.Vertical
                        width: 20
                        height: 100
                    }
                    PlasmaComponents.ProgressBar {
                        value: 0.4
                        orientation: Qt.Vertical
                        width: 20
                        height: 120
                    }
                    PlasmaComponents.ProgressBar {
                        orientation: Qt.Vertical
                        width: 20
                        height: 100
                        indeterminate: true
                    }
                }
            }
        }
    }

    PlasmaComponents.ScrollBar {
        id: horizontalScrollBar

        flickableItem: flickable
        orientation: Qt.Horizontal
        anchors {
            left: parent.left
            right: verticalScrollBar.left
            bottom: parent.bottom
        }
    }

    PlasmaComponents.ScrollBar {
        id: verticalScrollBar

        orientation: Qt.Vertical
        flickableItem: flickable
        anchors {
            top: parent.top
            right: parent.right
            bottom: horizontalScrollBar.top
        }
    }
}
