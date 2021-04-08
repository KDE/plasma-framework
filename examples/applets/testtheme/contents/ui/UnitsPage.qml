/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

// IconTab

Item {


    property int cheight: 48
    property int cwidth: themePage.width / 1.2

    id: themePage
    anchors {
        margins: PlasmaCore.Units.largeSpacing
    }

    Column {
        //anchors.fill: parent
        //spacing: PlasmaCore.Units.smallSpacing/2

        PlasmaExtras.Heading {
            width: parent.width
            elide: Text.ElideRight
            level: 1
            text: "Theme & DPI Test"
        }

        Row {
            PlasmaComponents.Button {
                text: "DPI"
                width: cwidth/2
                onClicked: {
                    print("DPI Button onClicked");
                    var d = PlasmaCore.Units.devicePixelRatio * 96;
                    dpilabel.text = "\t" + d;
                }
            }
            PlasmaComponents.Label {
                id: dpilabel
            }
        }
        Row {
            PlasmaComponents.Label {
                text: "PlasmaCore.Units.devicePixelRatio: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: PlasmaCore.Units.devicePixelRatio
            }
        }

        Row {
            PlasmaComponents.Label {
                text: "PlasmaCore.Units.gridUnit: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: PlasmaCore.Units.gridUnit
            }
        }

        Row {
            PlasmaComponents.Label {
                text: "PlasmaCore.Units.largeSpacing: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: PlasmaCore.Units.largeSpacing
            }
        }
        Row {
            PlasmaComponents.Label {
                text: "PlasmaCore.Units.smallSpacing: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: PlasmaCore.Units.smallSpacing
            }
        }

/*
        Row {
            PlasmaComponents.Label {
                text: "highlightColor: "
                width: cwidth
            }
            Rectangle {
                width: cheight
                height: cheight / 2
                color: PlasmaCore.Theme.highlightColor
            }
        }*/
    }
}
