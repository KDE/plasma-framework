/*
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

import QtQuick 1.0
import org.kde.plasma.components 0.1 as PlasmaComponents

PlasmaComponents.Page {
    height: childrenRect.height
    property int implicitHeight: childrenRect.height

    tools: PlasmaComponents.ToolBarLayout {
        spacing: 5
        PlasmaComponents.ToolButton {
            text: "ToolButton"
        }
        PlasmaComponents.ProgressBar {
            value: 0.3
        }
        PlasmaComponents.TextField {
            text: "Busy widgets"
        }
    }
    Column {
        spacing: 20

        Text {
            font.pixelSize: 20
            text: "Busy Indicator"
        }

        PlasmaComponents.BusyIndicator { }

        PlasmaComponents.BusyIndicator { running: true }

        Text {
            font.pixelSize: 20
            text: "Progress Bar"
        }

        Text { text: "Horizontal" }

        PlasmaComponents.ProgressBar {
            value: 0.3
        }

        PlasmaComponents.ProgressBar {
            indeterminate: true
        }

        PlasmaComponents.ProgressBar {
            minimumValue: 0
            maximumValue: 100
            value: 30
        }

        Text { text: "Vertical" }
        Row {
            spacing: 20
            PlasmaComponents.ProgressBar {
                value: 0.3
                orientation: Qt.Vertical
                width: 20
                height: 100
            }
            PlasmaComponents.ProgressBar {
                value: 0.4
                orientation: Qt.Vertical
                width: 20
                height: 120
            }
            PlasmaComponents.ProgressBar {
                orientation: Qt.Vertical
                width: 20
                height: 100
                indeterminate: true
            }
        }
    }
}
