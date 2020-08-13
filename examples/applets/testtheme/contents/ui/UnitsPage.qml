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
        margins: units.largeSpacing
    }

    Column {
        //anchors.fill: parent
        //spacing: units.smallSpacing/2

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
                    var d = units.devicePixelRatio * 96;
                    dpilabel.text = "\t" + d;
                }
            }
            PlasmaComponents.Label {
                id: dpilabel
            }
        }
        Row {
            PlasmaComponents.Label {
                text: "units.devicePixelRatio: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: units.devicePixelRatio
            }
        }

        Row {
            PlasmaComponents.Label {
                text: "units.gridUnit: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: units.gridUnit
            }
        }

        Row {
            PlasmaComponents.Label {
                text: "units.largeSpacing: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: units.largeSpacing
            }
        }
        Row {
            PlasmaComponents.Label {
                text: "units.smallSpacing: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: units.smallSpacing
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
                color: theme.highlightColor
            }
        }*/
    }
}
