/*
 *  Copyright 2013 Sebastian Kügler <sebas@kde.org>
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

// IconTab

PlasmaComponents.Page {
    id: themePage
    anchors {
        fill: parent
        margins: _s
    }
    Column {
        anchors.fill: parent
        spacing: _s

        PlasmaExtras.Title {
            width: parent.width
            elide: Text.ElideRight
            text: "Theme"
        }
        PlasmaComponents.Label {
            text: "This is the default Font."
            font: theme.smallestFont
            //font.pointSize: theme.smallestReadableFont.pointSize

        }
        PlasmaComponents.Label {
            text: "This is the smallest readable Font."
            font.pointSize: theme.smallestFont.pointSize

        }
    }
}
