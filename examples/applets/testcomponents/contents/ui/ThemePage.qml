/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

// IconTab

PlasmaComponents.Page {


    property int cheight: 48
    property int cwidth: themePage.width / 1.5

    id: themePage
    anchors {
        fill: parent
        margins: _s
    }

    Column {
        //anchors.fill: parent
        //spacing: _s / 4

        PlasmaExtras.Heading {
            width: parent.width
            elide: Text.ElideRight
            level: 1
            text: "Theme"
        }

        PlasmaComponents.Label {
            text: "This is the smallest readable Font."
            font: PlasmaCore.Theme.smallestFont

        }
        Row {
            PlasmaComponents.Label {
                text: "textColor: "
                width: cwidth
            }
            Rectangle {
                width: cheight
                height: cheight / 2
                color: PlasmaCore.Theme.textColor
            }
        }
        Row {
            PlasmaComponents.Label {
                text: "buttonTextColor: "
                width: cwidth
            }
            Rectangle {
                width: cheight
                height: cheight / 2
                color: buttonTextColor
            }
        }
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
        }
        Row {
            PlasmaComponents.Label {
                text: "viewBackgroundColor: "
                width: cwidth
            }
            Rectangle {
                width: cheight
                height: cheight / 2
                color: PlasmaCore.Theme.viewBackgroundColor
            }
        }
    }
}
