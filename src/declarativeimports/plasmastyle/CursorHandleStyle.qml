/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * \internal
 */
Item {
    width: PlasmaCore.Units.gridUnit
    height: styleData.lineHeight + PlasmaCore.Units.gridUnit
    visible: styleData.hasSelection

    Rectangle {
        id: handle
        width: PlasmaCore.Units.gridUnit
        height: width
        radius: width
        color: PlasmaCore.Theme.highlightColor
        anchors.bottom: parent.bottom
    }
    Rectangle {
        width: PlasmaCore.Units.gridUnit/2
        height: width
        anchors {
            left: handle.left
            top: handle.top
        }
        color: PlasmaCore.Theme.highlightColor
    }
    Rectangle {
        width: PlasmaCore.Units.smallSpacing
        height: styleData.lineHeight +  PlasmaCore.Units.gridUnit/2
        color: PlasmaCore.Theme.highlightColor
        anchors.left: parent.left
    }
}
