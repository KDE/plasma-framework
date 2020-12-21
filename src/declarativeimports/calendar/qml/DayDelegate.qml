/*
    SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import org.kde.plasma.calendar 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as Components

import org.kde.plasma.calendar 2.0

MouseArea {
    id: dayStyle

    hoverEnabled: true

    signal activated

    readonly property date thisDate: new Date(yearNumber, typeof monthNumber !== "undefined" ? monthNumber - 1 : 0, typeof dayNumber !== "undefined" ? dayNumber : 1)
    readonly property bool today: {
        var today = root.today;
        var result = true;
        if (dateMatchingPrecision >= Calendar.MatchYear) {
            result = result && today.getFullYear() === thisDate.getFullYear()
        }
        if (dateMatchingPrecision >= Calendar.MatchYearAndMonth) {
            result = result && today.getMonth() === thisDate.getMonth()
        }
        if (dateMatchingPrecision >= Calendar.MatchYearMonthAndDay) {
            result = result && today.getDate() === thisDate.getDate()
        }
        return result
    }
    readonly property bool selected: {
        var current = root.currentDate;
        var result = true;
        if (dateMatchingPrecision >= Calendar.MatchYear) {
            result = result && current.getFullYear() === thisDate.getFullYear()
        }
        if (dateMatchingPrecision >= Calendar.MatchYearAndMonth) {
            result = result && current.getMonth() === thisDate.getMonth()
        }
        if (dateMatchingPrecision >= Calendar.MatchYearMonthAndDay) {
            result = result && current.getDate() === thisDate.getDate()
        }
        return result
    }

    onHeightChanged: {
        // this is needed here as the text is first rendered, counting with the default root.cellHeight
        // then root.cellHeight actually changes to whatever it should be, but the Label does not pick
        // it up after that, so we need to change it explicitly after the cell size changes
        label.font.pixelSize = Math.max(PlasmaCore.Theme.smallestFont.pixelSize, Math.floor(daysCalendar.cellHeight / 3))
    }

    Rectangle {
        id: todayRect
        anchors.fill: parent
        opacity: {
            if (selected && today) {
                0.6
            } else if (today) {
                0.4
            } else {
                0
            }
        }
        color: PlasmaCore.Theme.textColor
    }

    Rectangle {
        id: highlightDate
        anchors.fill: todayRect
        opacity: {
            if (selected) {
                0.6
            } else if (dayStyle.containsMouse) {
                0.4
            } else {
                0
            }
        }
        visible: !today
        color: PlasmaCore.Theme.highlightColor
        z: todayRect.z - 1
    }

    Loader {
        active: model.containsMajorEventItems !== undefined && model.containsMajorEventItems
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        height: parent.height / 3
        width: height
        sourceComponent: eventsMarkerComponent
    }

    Components.Label {
        id: label
        anchors {
            fill: todayRect
            margins: PlasmaCore.Units.smallSpacing
        }
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        text: model.label || dayNumber
        opacity: isCurrent ? 1.0 : 0.5
        wrapMode: Text.NoWrap
        elide: Text.ElideRight
        fontSizeMode: Text.HorizontalFit
        font.pixelSize: Math.max(PlasmaCore.Theme.smallestFont.pixelSize, Math.floor(daysCalendar.cellHeight / 3))
        // Plasma component set point size, this code wants to set pixel size
        // Setting both results in a warning
        // -1 is an undocumented same as unset (see qquickvaluetypes)
        font.pointSize: -1
        color: today ? PlasmaCore.Theme.backgroundColor : PlasmaCore.Theme.textColor
    }
}
