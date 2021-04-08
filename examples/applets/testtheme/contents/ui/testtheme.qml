/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
// import org.kde.plasma.extras 2.0 as PlasmaExtras
// import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Item {
    id: root
    width: 300
    height: 400
    clip: true
    Layout.minimumWidth: PlasmaCore.Units.gridUnit * 10
    Layout.minimumHeight: PlasmaCore.Units.gridUnit * 10

    property int _s: PlasmaCore.Units.iconSizes.small
    property int _h: PlasmaCore.Units.iconSizes.medium

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

    PlasmaComponents.Label {
        id: sliderLabel
        anchors {
            bottom: parent.bottom
            left: parent.left
        }
        width: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 5
        text: "dpi: " + dprSlider.value
    }

    PlasmaComponents.Slider {
        id: dprSlider
        visible: false
        anchors {
            bottom: parent.bottom
            bottomMargin: PlasmaCore.Units.largeSpacing/2
            left: sliderLabel.right
            right: parent.right
        }
        minimumValue: 0
        maximumValue: 300
        stepSize: 20
        focus: true
        onValueChanged: {
            var r = value / 96;
            print("Setting PlasmaCore.Units.devicePixelRatio: " + r);
            PlasmaCore.Units.devicePixelRatio = r;
            if (value == 0) {
                value = PlasmaCore.Units.devicePixelRatio * 96;
            }
        }

        Component.onCompleted: dprSlider.value = PlasmaCore.Units.devicePixelRatio * 96
    }


    Component.onCompleted: {
        print("Components Test Applet loaded");
    }
}
