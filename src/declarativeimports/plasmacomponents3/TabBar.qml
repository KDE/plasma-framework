/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.TabBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    spacing: 0

    contentItem: ListView {
        implicitWidth: control.contentModel.count === 0 ? 0 : control.contentModel.get(0).implicitWidth * count + spacing * (count - 1)
        implicitHeight: control.contentModel.count === 0 ? 0 : control.contentModel.get(0).implicitHeight

        model: control.contentModel
        currentIndex: control.currentIndex

        spacing: control.spacing
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        flickableDirection: Flickable.AutoFlickIfNeeded
        snapMode: ListView.SnapToItem

        highlightMoveDuration: PlasmaCore.Units.longDuration
        highlightRangeMode: ListView.ApplyRange
        preferredHighlightBegin: 40
        preferredHighlightEnd: width - 40
        highlightResizeDuration: 0
        highlight: PlasmaCore.FrameSvgItem {
            imagePath: "widgets/tabbar"
            prefix: control.position == T.TabBar.Header ? "north-active-tab" : "south-active-tab"
            colorGroup: PlasmaCore.ColorScope.colorGroup
        }
    }
}
