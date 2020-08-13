/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Window 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

// DialogContent

Item {
    id: dialogsPage
    width: 300
    height: 200
    signal closeMe()
    Rectangle {
        color: "green"
        //anchors.margins: 24
        opacity: 0
        anchors.fill: parent
    }
    Column {
        anchors.fill: parent
        spacing: 12
        PlasmaExtras.Heading {
            id: tx
            level: 1
            text: "Test Dialog"
        }
        PlasmaComponents.TextArea {
            anchors { left: parent.left; right: parent.right; top: localeItem.bottom; }
            width: parent.width
            height: 80
        }
        PlasmaComponents.Button {
            id: thanks
            anchors { horizontalCenter: parent.horizontalCenter; bottom: parent.bottom; bottomMargin: 24; }
            iconSource: "dialog-ok"
            text: "Thanks."
            //onClicked: dialogsPage.parent.visible = false;
            onClicked: closeMe()
        }
    }
}

