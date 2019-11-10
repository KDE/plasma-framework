/*
 *   Copyright 2016 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

import QtQuick 2.1
import QtQuick.Layouts 1.2
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ColumnLayout {
    width: 600
    height: 600
    PlasmaCore.ColorScope {
         id: scope1
         colorGroup: PlasmaCore.Theme.NormalColorGroup
         Layout.minimumWidth: rect1.width
         Layout.minimumHeight: rect1.height
         Rectangle {
            id: rect1
            width: units.gridUnit * 30
            height: units.gridUnit * 15
            color: PlasmaCore.ColorScope.backgroundColor

            Column {
                id: widgetsParent
                anchors.centerIn: parent
                PlasmaCore.Svg {
                    id: audioSvg
                    imagePath: "icons/audio"
                    colorGroup: PlasmaCore.ColorScope.colorGroup
                }
                PlasmaCore.SvgItem {
                    width: 32
                    height: 32
                    elementId: "audio-volume-medium"
                    svg: audioSvg
                }
                Row {
                    PlasmaCore.IconItem {
                        colorGroup: PlasmaCore.ColorScope.colorGroup
                        source: "audio-volume-medium"
                    }
                    PlasmaComponents.Label {
                        text: "Some text"
                    }
                }
                Rectangle {
                    width: parent.width
                    height: units.gridUnit * 3
                    color: PlasmaCore.ColorScope.textColor
                }
            }
        }
    }
    PlasmaCore.ColorScope {
         id: scope2
         colorGroup: PlasmaCore.Theme.ComplementaryColorGroup
         Layout.minimumWidth: rect1.width
         Layout.minimumHeight: rect1.height
         Rectangle {
             id: rect2
             width: units.gridUnit * 30
             height: units.gridUnit * 15
             color: PlasmaCore.ColorScope.backgroundColor
         }
    }
    PlasmaComponents.Button {
        y: 300
        text: "switch"
        onClicked: widgetsParent.parent == rect2 ? widgetsParent.parent = rect1 : widgetsParent.parent = rect2
    }
}
