/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
