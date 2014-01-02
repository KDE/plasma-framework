// -*- coding: iso-8859-1 -*-
/*
 *   Copyright 2012 Marco Martin <mart@kde.org>
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

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Column {
    width: 500
    height: 500
    property int minimumWidth: 200
    property int minimumHeight: 300

    PlasmaCore.DataSource {
        id: source
        dataEngine: "org.kde.examples.sourcesOnRequest"
        interval: 1000
        connectedSources: "test"
    }
    
    PlasmaComponents.Label {
        text: source.data.test["Update Count"]
    }
    PlasmaExtras.ScrollArea {
        anchors {
            left: parent.left
            right: parent.right
        }
        height: 500
        ListView {
            model: source.models.test
            delegate: PlasmaComponents.Label {
                text: model.display
            }
        }
    }
}
