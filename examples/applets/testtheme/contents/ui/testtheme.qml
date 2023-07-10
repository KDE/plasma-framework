/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.kirigami 2.20 as Kirigami

Item {
    id: root
    width: 300
    height: 400
    clip: true
    Layout.minimumWidth: Kirigami.Units.gridUnit * 10
    Layout.minimumHeight: Kirigami.Units.gridUnit * 10

    property int _s: Kirigami.Units.iconSizes.small
    property int _h: Kirigami.Units.iconSizes.medium

    PlasmaComponents.TabBar {
        id: tabBar

        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            margins: _m
        }
        height: _h

        PlasmaComponents.TabButton { tab: fontsPage; text: i18n("Fonts"); iconSource: "preferences-desktop-font"}
        PlasmaComponents.TabButton { tab: scalePage; text: i18n("Scaling"); iconSource: "preferences-system-windows"}
        PlasmaComponents.TabButton { tab: unitsPage; text: i18n("Units"); iconSource: "preferences-desktop-appearance"}
    }

    PlasmaComponents.TabGroup {
        id: tabGroup
        anchors {
            left: parent.left
            right: parent.right
            top: tabBar.bottom
            bottom: dprSlider.top
        }

        FontsPage {
            id: fontsPage
        }

        ScalePage {
            id: scalePage
        }

        UnitsPage {
            id: unitsPage
        }

    }

    Component.onCompleted: {
        print("Components Test Applet loaded");
    }
}
