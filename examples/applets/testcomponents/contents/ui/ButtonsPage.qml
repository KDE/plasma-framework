/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.kirigami 2.20 as Kirigami

// ButtonsPage

PlasmaComponents.Page {
    id: plasmoidPage
    anchors {
        fill: parent
        margins: _s
    }
    Column {
        spacing: _s/2
        anchors.fill: parent
        Kirigami.Heading {
            level: 1
            width: parent.width
            text: "Buttons"
        }
        Row {
            height: _h
            spacing: _s
            PlasmaComponents.Button {
                text: "Button"
                iconSource: "call-start"
            }
            PlasmaComponents.ToolButton {
                text: "ToolButton"
                iconSource: "call-stop"
            }
        }
        Row {
            height: _h
            spacing: _s
            PlasmaComponents.RadioButton {
                id: radio
                text: "RadioButton"
                //iconSource: "call-stop"
                onCheckedChanged: if (checked) tfield.forceActiveFocus()
            }
            PlasmaComponents.TextField {
                id: tfield
                enabled: radio.checked
                text: "input here"
                clearButtonShown: true
            }
        }
//         PlasmaComponents.TextArea {
//             width: parent.width
//             height: _h*2
//         }
    }
}

