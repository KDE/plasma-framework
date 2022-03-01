/*
    SPDX-FileCopyrightText: 2014 Aleix Pol Gonzalez <aleixpol@blue-systems.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


import QtQuick 2.0
import QtQuick.Layouts 1.1
import QtQuick.Controls 2.15

import org.kde.plasma.core 2.0 as PlasmaCore

Item
{
    width: 500
    height: 500

    PlasmaCore.FrameSvgItem {
        id: theItem

        imagePath: "widgets/background"
        anchors {
            fill: parent
            margins: 10
        }

        Button {
            text: "left"
            checkable: true
            checked: true
            anchors {
                horizontalCenterOffset: -50
                centerIn: parent
            }
            onClicked: {
                if (checked)
                    theItem.enabledBorders |= PlasmaCore.FrameSvg.LeftBorder;
                else
                    theItem.enabledBorders &=~PlasmaCore.FrameSvg.LeftBorder;
            }
        }
        Button {
            text: "right"
            checkable: true
            checked: true

            anchors {
                horizontalCenterOffset: 50
                centerIn: parent
            }
            onClicked: {
                if (checked)
                    theItem.enabledBorders |= PlasmaCore.FrameSvg.RightBorder;
                else
                    theItem.enabledBorders &=~PlasmaCore.FrameSvg.RightBorder;
            }
        }
        Button {
            text: "top"
            checkable: true
            checked: true

            anchors {
                verticalCenterOffset: -50
                centerIn: parent
            }
            onClicked: {
                if (checked)
                    theItem.enabledBorders |= PlasmaCore.FrameSvg.TopBorder;
                else
                    theItem.enabledBorders &=~PlasmaCore.FrameSvg.TopBorder;
            }
        }
        Button {
            text: "bottom"
            checkable: true
            checked: true

            anchors {
                verticalCenterOffset: 50
                centerIn: parent
            }
            onClicked: {
                if (checked)
                    theItem.enabledBorders |= PlasmaCore.FrameSvg.BottomBorder;
                else
                    theItem.enabledBorders &=~PlasmaCore.FrameSvg.BottomBorder;
            }
        }
    }
}

