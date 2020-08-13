// -*- coding: iso-8859-1 -*-
/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Column {
    width: 500
    height: 500
    Layout.minimumWidth: 200
    Layout.minimumHeight: 300

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
