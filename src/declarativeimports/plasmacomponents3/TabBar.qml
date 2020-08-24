/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.TabBar {
    id: control

    implicitWidth: contentItem.implicitWidth
    implicitHeight: contentItem.implicitHeight

    spacing: 0

    contentItem: ListView {
        implicitWidth: control.contentModel.get(0).implicitWidth * count
        implicitHeight: control.contentModel.get(0).height

        model: control.contentModel
        currentIndex: control.currentIndex

        spacing: 0
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.AutoFlickIfNeeded
        snapMode: ListView.SnapToItem

        highlightMoveDuration: units.longDuration
        highlightRangeMode: ListView.ApplyRange
        preferredHighlightBegin: 40
        preferredHighlightEnd: width - 40
        highlight: PlasmaCore.FrameSvgItem {
            imagePath: "widgets/tabbar"
            prefix: control.position == T.TabBar.Header ? "north-active-tab" : "south-active-tab"
            colorGroup: PlasmaCore.ColorScope.colorGroup
        }
    }



    background: Item {}
}
