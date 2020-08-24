/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

import org.kde.draganddrop 2.0 as DragAndDrop


//Item {
PlasmaComponents.ListItem {

    width: parent.width
    //height: _h * 1.5
    property alias icon: itemIcon.source
    property alias text: label.text

    PlasmaCore.IconItem {
        id: itemIcon

        width: _h
        height: width
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: _h/2
    }

    PlasmaComponents.Label {
        id: label

        anchors {
            verticalCenter: parent.verticalCenter
            left: itemIcon.right
            leftMargin: _h/2
            right: parent.right
            rightMargin: _h/2
        }

    }
}
