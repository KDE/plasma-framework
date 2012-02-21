// -*- coding: iso-8859-1 -*-
/*
 *   Author: Giorgos Tsiapaliwkas <terietor@gmail.com>
 *   Date: Wed Feb 15 2012, 18:28:32
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.locale 0.1

Item {
    id: root
    property int minimumHeight:200
    property int minimumWidth:500
Column {
    id: column
    anchors.horizontalCenter: root.horizontalCenter
    spacing: 20
    Text {
        id: text
        anchors.horizontalCenter: column.horizontalCenter
        text: "<B>This is a test plasmoid for the locale bindings</B>"
        color: "red"
    }
    PlasmaComponents.Button {
        id: bt1
        anchors.horizontalCenter: column.horizontalCenter
        text: "click in order to test the Locale component"
        onClicked:{
            print("hello");
        }
    }
    PlasmaComponents.Button {
        id: bt2
        anchors.horizontalCenter: column.horizontalCenter
        text: "click in order to test the CalendarSystem component"
        onClicked:{
            print("hello again");
        }
    }
    Locale {
        id: locale
    }

    CalendarSystem {
        id: calendar
    }
}
}
