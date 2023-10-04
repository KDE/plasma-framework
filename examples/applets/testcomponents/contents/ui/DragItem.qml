/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick

import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

import org.kde.draganddrop as DragAndDrop

PlasmaComponents.ItemDelegate {
    id: control

    width: parent?.width ?? 0
    //height: _h * 1.5

    Kirigami.Icon {
        id: itemIcon

        width: _h
        height: width
        anchors.verticalCenter: parent.verticalCenter
        anchors.left: parent.left
        anchors.leftMargin: _h/2

        source: control.icon.name
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

        text: control.text
    }
}
