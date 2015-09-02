/*
*   Copyright (C) 2015 by Marco MArtin <mart@kde.org>
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
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    width: units.gridUnit
    height: styleData.lineHeight + units.gridUnit
    visible: styleData.hasSelection

    Rectangle {
        id: handle
        width: units.gridUnit
        height: width
        radius: width
        color: theme.highlightColor
        anchors.bottom: parent.bottom
    }
    Rectangle {
        width: units.gridUnit/2
        height: width
        anchors {
            left: handle.left
            top: handle.top
        }
        color: theme.highlightColor
    }
    Rectangle {
        width: units.smallSpacing
        height: styleData.lineHeight +  units.gridUnit/2
        color: theme.highlightColor
        anchors.left: parent.left
    }
}
