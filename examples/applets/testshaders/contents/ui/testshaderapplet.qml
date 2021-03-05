/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Item {
    id: root
    width: 400
    height: 400

    Layout.minimumWidth: PlasmaCore.Units.gridUnit * 20
    Layout.minimumHeight: PlasmaCore.Units.gridUnit * 30
    property int _s: PlasmaCore.Units.iconSizes.small
    property int _h: PlasmaCore.Units.iconSizes.desktop
    property int _m: 12

    Item {
        id: mainItem
        anchors.fill: parent

        PlasmaComponents.TabBar {
            id: tabBar

            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            height: PlasmaCore.Units.iconSizes.toolbar * 1.5

            PlasmaComponents.TabButton { tab: colorShower; text: tab.pageName; }
            PlasmaComponents.TabButton { tab: wobbleExample; text: tab.pageName; }
            PlasmaComponents.TabButton { tab: shadowExample; text: tab.pageName; }
            PlasmaComponents.TabButton { tab: simpleExample; text: tab.pageName; }
            //PlasmaComponents.TabButton { tab: vertexPage; iconSource: vertexPage.icon; }
        }

        PlasmaComponents.TabGroup {
            id: tabGroup
            anchors {
                left: parent.left
                right: parent.right
                top: tabBar.bottom
                bottom: parent.bottom
            }

            ColorShower {
                id: colorShower
            }
            WobbleExample {
                id: wobbleExample
            }
//             ColorExample {
//                 id: colorExample
//             }
            Shadows {
                id: shadowExample
            }
            SimpleExample {
                id: simpleExample
            }
//             EditorPage {
//                 id: vertexPage
//             }
        }
    }

    Component.onCompleted: {
        print("Shader Test Applet loaded");
    }
}
