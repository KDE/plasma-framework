/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.plasma5support 2.0 as P5Support
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons
import org.kde.kirigami 2.20 as Kirigami

Item {
    id: root
    width: 100
    height: 100
    clip: true
    Layout.minimumWidth: Kirigami.Units.gridUnit * 20
    Layout.minimumHeight: Kirigami.Units.gridUnit * 30

    property int _s: Kirigami.Units.iconSizes.small
    property int _h: Kirigami.Units.iconSizes.desktop

    P5Support.DataSource {
        id: dataSource
    }

    PlasmaComponents.TabBar {
        id: tabBar

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
        }
        height: _h

        PlasmaComponents.TabButton { tab: themePage; iconSource: "preferences-desktop-appearance"}
        PlasmaComponents.TabButton { tab: dragPage; iconSource: "preferences-desktop-mouse"}
        PlasmaComponents.TabButton { tab: iconsPage; iconSource: "preferences-desktop-icons"}
        PlasmaComponents.TabButton { tab: dialogsPage; iconSource: "preferences-system-windows"}
        PlasmaComponents.TabButton { tab: buttonsPage; iconSource: "preferences-desktop-theme"}
        PlasmaComponents.TabButton { tab: plasmoidPage; iconSource: "plasma"}
        PlasmaComponents.TabButton { tab: mousePage; iconSource: "preferences-desktop-mouse"}
    }

    PlasmaComponents.TabGroup {
        id: tabGroup
        anchors {
            left: parent.left
            right: parent.right
            top: tabBar.bottom
            bottom: parent.bottom
        }

        //currentTab: tabBar.currentTab

        ThemePage {
            id: themePage
        }
        DragPage {
            id: dragPage
        }
        IconsPage {
            id: iconsPage
        }
        DialogsPage {
            id: dialogsPage
        }
        ButtonsPage {
            id: buttonsPage
        }
        PlasmoidPage {
            id: plasmoidPage
        }

        MousePage {
            id: mousePage
        }

    }

    Component.onCompleted: {
        print("Components Test Applet loaded");
        //dataSource.engine = "org.kde.foobar"
//         tabGroup.currentTab = mousePage;
    }
}
