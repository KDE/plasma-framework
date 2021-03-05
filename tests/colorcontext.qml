/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
            width: PlasmaCore.Units.gridUnit * 30
            height: PlasmaCore.Units.gridUnit * 15
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
                    height: PlasmaCore.Units.gridUnit * 3
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
             width: PlasmaCore.Units.gridUnit * 30
             height: PlasmaCore.Units.gridUnit * 15
             color: PlasmaCore.ColorScope.backgroundColor
         }
    }
    PlasmaComponents.Button {
        y: 300
        text: "switch"
        onClicked: widgetsParent.parent == rect2 ? widgetsParent.parent = rect1 : widgetsParent.parent = rect2
    }
}
