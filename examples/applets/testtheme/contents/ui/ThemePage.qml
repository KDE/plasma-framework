/*
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.qtextracomponents 2.0 as QtExtras

// IconTab

Item {


    property int cheight: 48
    property int cwidth: themePage.width / 1.2

    id: themePage
    anchors {
        margins: theme.largeSpacing
    }

    Column {
        //anchors.fill: parent
        //spacing: theme.smallSpacing/2

        PlasmaExtras.Title {
            width: parent.width
            elide: Text.ElideRight
            text: "Theme & DPI Test"
        }

        Row {
            PlasmaComponents.Button {
                text: "DPI"
                width: cwidth/2
                onClicked: {
                    print("DPI Button onClicked");
                    print(units.gridUnit);
                    var d = theme.dpi(dpilabel);
                    dpilabel.text = "\t" + d
                }
            }
            PlasmaComponents.Label {
                id: dpilabel
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
                text: "units.dp: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: units.dp(1)
            }
        }

        Row {
            PlasmaComponents.Label {
                text: "units.gu: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: units.gu(1)
            }
        }

        Row {
            PlasmaComponents.Label {
                text: "theme.largeSpacing: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: theme.largeSpacing
            }
        }
        Row {
            PlasmaComponents.Label {
                text: "theme.smallSpacing: "
                width: cwidth
            }
            PlasmaComponents.Label {
                width: cheight
                height: cheight / 2
                text: theme.smallSpacing
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
