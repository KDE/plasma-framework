/*
    SPDX-FileCopyrightText: 2013 Heena Mahour <heena393@gmail.com>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2015 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
import QtQuick 2.0
import QtQuick.Controls 2.15 as QQC2
import QtQuick.Layouts 1.1

import org.kde.plasma.calendar 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as Components
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    id: root

    anchors.fill: parent

    property alias selectedMonth: calendarBackend.monthName
    property alias selectedYear: calendarBackend.year
    property alias firstDayOfWeek: calendarBackend.firstDayOfWeek

    property QtObject date
    property date currentDate

    property date showDate: new Date()

    property int borderWidth: 1
    property real borderOpacity: 0.4

    property int columns: calendarBackend.days
    property int rows: calendarBackend.weeks

    property Item selectedItem
    property int week;
    property int firstDay: new Date(showDate.getFullYear(), showDate.getMonth(), 1).getDay()
    property alias today: calendarBackend.today
    property bool showWeekNumbers: false
    property bool showCustomHeader: false

    /**
     * Current index of the internal swipeView. Usefull for binding
     * a TabBar to it.
     */
    property alias currentIndex: swipeView.currentIndex

    property alias cellHeight: mainDaysCalendar.cellHeight
    property QtObject daysModel: calendarBackend.daysModel

    function isToday(date) {
        return date.toDateString() === new Date().toDateString();
    }

    function eventDate(yearNumber,monthNumber,dayNumber) {
        const d = new Date(yearNumber, monthNumber-1, dayNumber);
        return Qt.formatDate(d, "dddd dd MMM yyyy");
    }

    /**
     * Move calendar to month view showing today's date.
     */
    function resetToToday() {
        calendarBackend.resetToToday();
        root.currentDate = root.today;
        swipeView.currentIndex = 0;
    }

    function updateYearOverview() {
        const date = calendarBackend.displayedDate;
        const day = date.getDate();
        const year = date.getFullYear();

        for (let i = 0, j = monthModel.count; i < j; ++i) {
            monthModel.setProperty(i, "yearNumber", year);
        }
    }

    function updateDecadeOverview() {
        const date = calendarBackend.displayedDate;
        const day = date.getDate();
        const month = date.getMonth() + 1;
        const year = date.getFullYear();
        const decade = year - year % 10;

        for (let i = 0, j = yearModel.count; i < j; ++i) {
            const label = decade - 1 + i;
            yearModel.setProperty(i, "yearNumber", label);
            yearModel.setProperty(i, "label", label);
        }
    }

    /**
     * Possible calendar views
     */
    enum CalendarView {
        DayView,
        MonthView,
        YearView
    }

    /**
     * Go to the next month/year/decade depending on the current
     * calendar view displayed.
     */
    function nextFrame() {
        if (swipeView.currentIndex === 0) {
            calendarBackend.nextMonth();
        } else if (swipeView.currentIndex === 1) {
            calendarBackend.nextYear();
        } else if (swipeView.currentIndex === 2) {
            calendarBackend.nextDecade();
        }
    }

    /**
     * Go to the previous month/year/decade depending on the current
     * calendar view displayed.
     */
    function previousFrame() {
        if (swipeView.currentIndex === 0) {
            calendarBackend.previousMonth();
        } else if (swipeView.currentIndex === 1) {
            calendarBackend.previousYear();
        } else if (swipeView.currentIndex === 2) {
            calendarBackend.previousDecade();
        }
    }

    /**
     * \return CalendarView
     */
    readonly property var calendarViewDisplayed: {
        if (swipeView.currentIndex === 0) {
            return MonthView.CalendarView.DayView;
        } else if (swipeView.currentIndex === 1) {
            return MonthView.CalendarView.MonthView;
        } else if (swipeView.currentIndex === 2) {
            return MonthView.CalendarView.YearView;
        }
    }

    /**
     * Show month view.
     */
    function showMonthView() {
        swipeView.currentIndex = 0;
    }

    /**
     * Show year view.
     */
    function showYearView() {
        swipeView.currentIndex = 1;
    }

    /**
     * Show month view.
     */
    function showDecadeView() {
        swipeView.currentIndex = 2;
    }

    Calendar {
        id: calendarBackend

        days: 7
        weeks: 6
        firstDayOfWeek: Qt.locale().firstDayOfWeek
        today: root.today

        Component.onCompleted: {
            daysModel.setPluginsManager(EventPluginsManager);
        }

        onYearChanged: {
            updateYearOverview()
            updateDecadeOverview()
        }
    }

    ListModel {
        id: monthModel

        Component.onCompleted: {
            for (var i = 0; i < 12; ++i) {
                append({
                    label: Qt.locale(Qt.locale().uiLanguages[0]).standaloneMonthName(i, Locale.LongFormat),
                    monthNumber: i + 1,
                    isCurrent: true
                })
            }
            updateYearOverview()
        }
    }

    ListModel {
        id: yearModel

        Component.onCompleted: {
            for (var i = 0; i < 12; ++i) {
                append({
                    label: 2050, // this value will be overwritten, but it set the type of the property to int
                    yearNumber: 2050,
                    isCurrent: (i > 0 && i < 11) // first and last year are outside the decade
                })
            }
            updateDecadeOverview()
        }
    }

    // NOTE: this MouseArea is a workaround on a PinchArea quirk:
    // When the pich is done spanning multiple child mouseareas it on't work:
    // a MouseArea on top of all the child mouseareas that just refuses events makes the pincharea work.
    // BUG: https://bugreports.qt.io/browse/QTBUG-76569
    MouseArea {
        anchors.fill: parent
        z: 1
        onPressed: mouse.accepted = false
    }

    ColumnLayout {
        id: viewHeader
        visible: !showCustomHeader
        height: !visible ? 0 : implicitHeight
        width: parent.width
        anchors {
            top: parent.top
        }

        RowLayout {
            PlasmaExtras.Heading {
                id: heading
                text: i18ndc("libplasma5", "Format: **month** year", "<strong>%1</strong> %2", root.selectedMonth, root.selectedYear.toString())

                level: 2
                elide: Text.ElideRight
                font.capitalization: Font.Capitalize
                //SEE QTBUG-58307
                //try to make all heights an even number, otherwise the layout engine gets confused
                Layout.preferredHeight: implicitHeight + implicitHeight%2
                Layout.fillWidth: true
            }
            Row {
                spacing: 0
                Components.ToolButton {
                    id: previousButton
                    property string tooltip: {
                        switch(root.calendarViewDisplayed) {
                            case MonthView.CalendarView.DayView:
                                return i18nd("libplasma5", "Previous Month")
                            case MonthView.CalendarView.MonthView:
                                return i18nd("libplasma5", "Previous Year")
                            case MonthView.CalendarView.YearView:
                                return i18nd("libplasma5", "Previous Decade")
                            default:
                                return "";
                        }
                    }

                    //SEE QTBUG-58307
                    Layout.preferredHeight: implicitHeight + implicitHeight % 2
                    icon.name: Qt.application.layoutDirection === Qt.RightToLeft ? "go-next" : "go-previous"
                    onClicked: root.previousFrame()
                    Accessible.name: tooltip
                    Components.ToolTip { text: parent.tooltip }
                }

                Components.ToolButton {
                    icon.name: "go-jump-today"
                    property string tooltip

                    //SEE QTBUG-58307
                    Layout.preferredHeight: implicitHeight + implicitHeight % 2
                    onClicked: root.resetToToday()
                    Components.ToolTip {
                        text: i18ndc("libplasma5", "Reset calendar to today", "Today")
                    }
                    Accessible.name: tooltip
                    Accessible.description: i18nd("libplasma5", "Reset calendar to today")
                }

                Components.ToolButton {
                    id: nextButton
                    property string tooltip: {
                        switch(root.calendarViewDisplayed) {
                            case MonthView.CalendarView.DayView:
                                return i18nd("libplasma5", "Next Month")
                            case MonthView.CalendarView.MonthView:
                                return i18nd("libplasma5", "Next Year")
                            case MonthView.CalendarView.YearView:
                                return i18nd("libplasma5", "Next Decade")
                            default:
                                return "";
                        }
                    }

                    //SEE QTBUG-58307
                    Layout.preferredHeight: implicitHeight + implicitHeight % 2
                    icon.name: Qt.application.layoutDirection === Qt.RightToLeft ? "go-previous" : "go-next"
                    Components.ToolTip { text: parent.tooltip }
                    onClicked: root.nextFrame();
                    Accessible.name: tooltip
                }
            }
        }

        Components.TabBar {
            id: tabBar
            currentIndex: swipeView.currentIndex
            Layout.preferredWidth: contentWidth
            Layout.alignment: Qt.AlignRight
            Components.TabButton {
                text: i18nc("libplasma5", "Days");
                onClicked: root.showMonthView();
                opacity: root.calendarViewDisplayed === MonthView.CalendarView.MonthView ? 1 : 0.8
                width: implicitWidth
            }
            Components.TabButton {
                text: i18nd("libplasma5", "Months");
                onClicked: root.showYearView();
                opacity: root.calendarViewDisplayed === MonthView.CalendarView.YearView ? 1 : 0.7
                width: implicitWidth
            }
            Components.TabButton {
                text: i18nd("libplasma5", "Years");
                onClicked: root.showDecadeView();
                opacity: root.calendarViewDisplayed === MonthView.CalendarView.DecadeView ? 1 : 0.7
                width: implicitWidth
            }
        }
    }

    QQC2.SwipeView {
        id: swipeView
        orientation: Qt.Vertical
        anchors {
            top: viewHeader.bottom
            left: parent.left
            right: parent.right
            bottom: parent.bottom
        }
        clip: true

        onCurrentIndexChanged: if (currentIndex > 1) {
            updateDecadeOverview();
        }

        // MonthView
        DaysCalendar {
            id: mainDaysCalendar

            columns: calendarBackend.days
            rows: calendarBackend.weeks

            showWeekNumbers: root.showWeekNumbers

            headerModel: calendarBackend.days
            gridModel: calendarBackend.daysModel

            dateMatchingPrecision: Calendar.MatchYearMonthAndDay

            onActivated: {
                const rowNumber = Math.floor(index / columns);
                week = 1 + calendarBackend.weeksModel[rowNumber];
                root.currentDate = new Date(date.yearNumber, date.monthNumber - 1, date.dayNumber)
            }
        }

        // YearView
        DaysCalendar {
            columns: 3
            rows: 4

            dateMatchingPrecision: Calendar.MatchYearAndMonth

            gridModel: monthModel
            onActivated: {
                calendarBackend.goToMonth(date.monthNumber);
                swipeView.currentIndex = 0;
            }
        }

        // DecadeView
        DaysCalendar {
            readonly property int decade: {
                const year = calendarBackend.displayedDate.getFullYear()
                return year - year % 10
            }

            columns: 3
            rows: 4

            dateMatchingPrecision: Calendar.MatchYear

            gridModel: yearModel
            onActivated: {
                calendarBackend.goToYear(date.yearNumber);
                swipeView.currentIndex = 1;
            }
        }
    }

    Component.onCompleted: {
        root.currentDate = calendarBackend.today
    }
}
