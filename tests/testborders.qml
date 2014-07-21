/*
 *   Copyright (C) 2014 by Aleix Pol Gonzalez <aleixpol@blue-systems.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */


import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

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

        PlasmaComponents.Button {
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
        PlasmaComponents.Button {
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
        PlasmaComponents.Button {
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
        PlasmaComponents.Button {
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

