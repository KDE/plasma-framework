/*
 *  Copyright 2013 Marco Martin <mart@kde.org>
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
import org.kde.plasma.plasmoid 2.0

Item {
    id: root
    width: 640
    height: 480

    property Item toolBox

    function addApplet(applet, x, y) {
        var component = Qt.createComponent("PlasmoidContainer.qml")
        var plasmoidContainer = component.createObject(root, {"x": x, "y": y});

        plasmoidContainer.parent = root;
        plasmoidContainer.applet = applet
        applet.parent = plasmoidContainer
        applet.anchors.fill = plasmoidContainer
        applet.visible = true
        plasmoidContainer.visible = true

    }

    Containment.onAppletAdded: {
        addApplet(applet, x, y);
    }

    Component.onCompleted: {
        print("Test Containment loaded")
        print(plasmoid)
    }
}
