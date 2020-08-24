// -*- coding: iso-8859-1 -*-
/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    Layout.minimumWidth: 200
    Layout.minimumHeight: 300
    Plasmoid.compactRepresentation: Component {
        PlasmaComponents.Button {
            text: i18n("Click me")
            onClicked: plasmoid.expanded = !plasmoid.expanded
        }
    }

    PlasmaComponents.Label {
        text: i18n("Hello world")
    }
}
