/*
 *  Copyright 2013 Heena Mahour <heena393@gmail.com>
 *  Copyright 2013 Martin Klapetek <mklapetek@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaComponents.Menu {
    id: testMenu
    property date today: new Date()

    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(0))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 0, 1)
    }

    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(1))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 1, 1)
    }

    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(2))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 2, 1)
    }
    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(3))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 3, 1)
    }
    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(4))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 4, 1)
    }
    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(5))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 5, 1)
    }
    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(6))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 6, 1)
    }
    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(7))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 7, 1)
    }
    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(8))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 8, 1)
    }
    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(9))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 9, 1)
    }
    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(10))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 10, 1)
    }
    PlasmaComponents.MenuItem {
        text: capitalizeFirstLetter(Qt.locale().standaloneMonthName(11))
        onClicked: calendarBackend.displayedDate = new Date(today.getFullYear(), 11, 1)
    }

    // Because some locales don't have it in standaloneMonthNames,
    // but we want our GUI to be pretty and want capitalization always
    function capitalizeFirstLetter(monthName) {
        return monthName.charAt(0).toUpperCase() + monthName.slice(1);
    }
}
