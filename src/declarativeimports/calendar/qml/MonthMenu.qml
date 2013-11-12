/*
 *  Copyright 2013 Heena Mahour <heena393@gmail.com>
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
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.qtextracomponents 2.0 as QtExtras

PlasmaComponents.Menu {
    id: testMenu

    PlasmaComponents.MenuItem {
        text: "January"
        onClicked: monthCalendar.startDate="2013-01-01"
    }

    PlasmaComponents.MenuItem {
        text: "February"
        onClicked: monthCalendar.startDate="2013-02-01"
    }

    PlasmaComponents.MenuItem {
        text: "March"
        onClicked: monthCalendar.startDate="2013-03-01"
    }
    PlasmaComponents.MenuItem {
        text: "April"
        onClicked: monthCalendar.startDate="2013-04-01"
    }
    PlasmaComponents.MenuItem {
        text: "May"
        onClicked: monthCalendar.startDate="2013-05-01"
    }
    PlasmaComponents.MenuItem {
        text: "June"
        onClicked: monthCalendar.startDate="2013-06-01"
    }
    PlasmaComponents.MenuItem {
        text: "July"
        onClicked: monthCalendar.startDate="2013-07-01"
    }
    PlasmaComponents.MenuItem {
        text: "August"
        onClicked: monthCalendar.startDate="2013-08-01"
    }
    PlasmaComponents.MenuItem {
        text: "September"
        onClicked: monthCalendar.startDate="2013-09-01"
    }
    PlasmaComponents.MenuItem {
        text: "October"
        onClicked: monthCalendar.startDate="2013-10-01"
    }
    PlasmaComponents.MenuItem {
        text: "November"
        onClicked: monthCalendar.startDate="2013-11-01"
    }
    PlasmaComponents.MenuItem {
        text: "December"
        onClicked: monthCalendar.startDate="2013-12-01"
    }
    Component.onCompleted:{
        print("TestMenu.qml served .. opening");
    }
}
 
