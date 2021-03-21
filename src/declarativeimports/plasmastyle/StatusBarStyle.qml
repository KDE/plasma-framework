/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls.Styles 1.2 as QtQuickControlStyle
import QtQuick.Controls 1.2
import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * \internal
 */
QtQuickControlStyle.StatusBarStyle {

    padding {
        left: PlasmaCore.Units.smallSpacing
        right: PlasmaCore.Units.smallSpacing
        top: PlasmaCore.Units.smallSpacing
        bottom: PlasmaCore.Units.smallSpacing/2
    }


    background: Item {
        implicitHeight: 16
        implicitWidth: 200

        Rectangle {
            anchors.top: parent.top
            width: parent.width
            height: 1
            color: PlasmaCore.ColorScope.textColor
            opacity: 0.1
        }
    }
}
