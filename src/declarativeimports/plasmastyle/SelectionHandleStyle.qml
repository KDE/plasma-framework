/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    width: PlasmaCore.Units.gridUnit
    x: -width
    height: styleData.lineHeight + PlasmaCore.Units.gridUnit
    visible: styleData.hasSelection

    Rectangle {
        id: handle
        width: PlasmaCore.Units.gridUnit
        height: width
        radius: width
        color: theme.highlightColor
        anchors.bottom: parent.bottom
    }
    Rectangle {
        width: PlasmaCore.Units.gridUnit/2
        height: width
        anchors {
            right: handle.right
            top: handle.top
        }
        color: theme.highlightColor
    }
    Rectangle {
        width: PlasmaCore.Units.smallSpacing
        height: styleData.lineHeight +  PlasmaCore.Units.gridUnit/2
        color: theme.highlightColor
        anchors.right: parent.right
    }
}
