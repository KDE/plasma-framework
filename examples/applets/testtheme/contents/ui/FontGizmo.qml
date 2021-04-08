/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Window 2.2
import QtQuick.Controls 1.0
import QtQuick.Controls.Private 1.0 as QtQuickControlsPrivate

import org.kde.plasma.core 2.0 as PlasmaCore
//import org.kde.plasma.components 2.0 as PlasmaComponents
//import org.kde.plasma.extras 2.0 as PlasmaExtras
//import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

Text {

    // Work around Qt bug where NativeRendering breaks for non-integer scale factors
    // https://bugreports.qt.io/browse/QTBUG-70481
    renderType: QtQuickControlsPrivate.Settings.isMobile || Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering
    font.pointSize: 22

    //font.family: PlasmaCore.Theme.defaultFont.family
    font.family: fontCheck.text
    font.weight: lightCheck.checked ? Font.Light : Font.Normal
    text: "Lesley 40:83 - (" + font.family + ")"
    height: paintedHeightCheck.checked ? paintedHeight : 22
    //anchors.fill: parent
    //spacing: PlasmaCore.Units.smallSpacing/2
    verticalAlignment: Text.AlignTop

    Rectangle {
        color: "yellow"
        visible: boxesCheck.checked

        height: 1
        width: paintedWidth

        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
        }
    }
    Rectangle {
        color: "transparent"
        border.width: 1
        border.color: "green"
        visible: boxesCheck.checked

        height: parent.paintedHeight
        width: paintedWidth

        anchors {
            top: parent.top
            left: parent.left
        }
    }
}
