/*
    SPDX-FileCopyrightText: 2014 Digia Plc and /or its subsidiary(-ies).
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

/**
 * \internal
 */
QtQuickControlStyle.CalendarStyle {
    id: styleRoot

    gridColor: Qt.rgba(PlasmaCore.ColorScope.textColor.r, PlasmaCore.ColorScope.textColor.g, PlasmaCore.ColorScope.textColor.b, 0.4)

    gridVisible: true


    background: Item {
        implicitWidth: Math.max(250, Math.round(PlasmaCore.Units.gridUnit * 14))
        implicitHeight: Math.max(250, Math.round(PlasmaCore.Units.gridUnit * 14))
    }

    navigationBar: Item {
        height: Math.round(PlasmaCore.Units.gridUnit * 2.73)

        PlasmaComponents.ToolButton {
            id: previousMonth
            width: parent.height
            height: width
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            iconSource: "go-previous"
            onClicked: control.showPreviousMonth()
        }
        PlasmaExtras.Heading {
            id: dateText
            text: styleData.title
            elide: Text.ElideRight
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: PlasmaCore.Units.gridUnit * 1.25
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: previousMonth.right
            anchors.leftMargin: 2
            anchors.right: nextMonth.left
            anchors.rightMargin: 2
        }
        PlasmaComponents.ToolButton {
            id: nextMonth
            width: parent.height
            height: width
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            iconSource: "go-next"
            onClicked: control.showNextMonth()
        }
    }

    dayDelegate: Rectangle {
        //FIXME: this stuff should depend from localization
        property int daysInAWeek: 7
        property int weeksOnACalendarMonth: 6
        anchors.fill: parent
        anchors.leftMargin: (!addExtraMargin || control.weekNumbersVisible) && styleData.index % daysInAWeek === 0 ? 0 : -1
        anchors.rightMargin: !addExtraMargin && styleData.index % daysInAWeek === daysInAWeek - 1 ? 0 : -1
        anchors.bottomMargin: !addExtraMargin && styleData.index >= daysInAWeek * (weeksOnACalendarMonth - 1) ? 0 : -1
        anchors.topMargin: styleData.selected ? -1 : 0
        color: styleData.date !== undefined && styleData.selected ? selectedDateColor : "transparent"

        readonly property bool addExtraMargin: control.frameVisible && styleData.selected
        readonly property color sameMonthDateTextColor: PlasmaCore.ColorScope.textColor
        readonly property color selectedDateColor: PlasmaCore.Theme.highlightColor
        readonly property color selectedDateTextColor: PlasmaCore.ColorScope.textColor
        readonly property color differentMonthDateTextColor: Qt.rgba(PlasmaCore.ColorScope.textColor.r, PlasmaCore.ColorScope.textColor.g, PlasmaCore.ColorScope.textColor.b, 0.4)
        readonly property color invalidDateColor: Qt.rgba(PlasmaCore.ColorScope.textColor.r, PlasmaCore.ColorScope.textColor.g, PlasmaCore.ColorScope.textColor.b, 0.2)
        PlasmaComponents.Label {
            id: dayDelegateText
            text: styleData.date.getDate()
            anchors.centerIn: parent
            horizontalAlignment: Text.AlignRight
            font.pixelSize: Math.min(parent.height/3, parent.width/3)
            color: {
                var theColor = invalidDateColor;
                if (styleData.valid) {
                    // Date is within the valid range.
                    theColor = styleData.visibleMonth ? sameMonthDateTextColor : differentMonthDateTextColor;
                    if (styleData.selected)
                        theColor = selectedDateTextColor;
                }
                theColor;
            }
        }
    }

    dayOfWeekDelegate: Item {
        implicitHeight: Math.round(PlasmaCore.Units.gridUnit * 2.25)
        PlasmaComponents.Label {
            text: control.__locale.dayName(styleData.dayOfWeek, control.dayOfWeekFormat)
            anchors.centerIn: parent
        }
    }

    weekNumberDelegate: Item {
        implicitWidth: Math.round(PlasmaCore.Units.gridUnit * 2)
        PlasmaComponents.Label {
            text: styleData.weekNumber
            anchors.centerIn: parent
        }
    }
}
