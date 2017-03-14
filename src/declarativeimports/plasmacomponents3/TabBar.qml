/*
 *   Copyright 2016 Marco Martin <mart@kde.org>
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.6
import QtQuick.Templates 2.0 as T
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
