/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.kirigami 2.20 as Kirigami

Item {


    property int cheight: 48
    property int cwidth: themePage.width / 1.2

    id: themePage
    anchors {
        margins: Kirigami.Units.largeSpacing
    }

    Column {
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
                    var d = 96;
                    dpilabel.text = "\t" + d;
                }
            }
            PlasmaComponents.Label {
                id: dpilabel
            }
        }
        Row {
            PlasmaComponents.Label {
                text: "Kirigami.Units.devicePixelRatio: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: Kirigami.Units.devicePixelRatio
            }
        }

        Row {
            PlasmaComponents.Label {
                text: "Kirigami.Units.gridUnit: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: Kirigami.Units.gridUnit
            }
        }

        Row {
            PlasmaComponents.Label {
                text: "Kirigami.Units.largeSpacing: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: Kirigami.Units.largeSpacing
            }
        }
        Row {
            PlasmaComponents.Label {
                text: "Kirigami.Units.smallSpacing: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: Kirigami.Units.smallSpacing
            }
        }
    }
}
