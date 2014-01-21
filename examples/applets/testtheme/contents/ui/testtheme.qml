/*
 *  Copyright 2014 Sebastian Kügler <sebas@kde.org>
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

import QtQuick 2.1

import org.kde.plasma.core 2.0 as PlasmaCore
// import org.kde.plasma.components 2.0 as PlasmaComponents
// import org.kde.plasma.extras 2.0 as PlasmaExtras
// import org.kde.qtextracomponents 2.0 as QtExtras

Item {
    id: root
    width: 300
    height: 400
    clip: true
    property int minimumWidth: units.gridUnit * 10
    property int minimumHeight: units.gridUnit * 10

    property int _s: units.iconSizes.small
    property int _h: units.iconSizes.desktop

    ThemePage {
        id: themePage
        anchors.fill: parent
    }

    Component.onCompleted: {
        print("Components Test Applet loaded");
    }
}