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
QtQuickControlStyle.SwitchStyle {
    id: styleRoot

    handle: PlasmaCore.FrameSvgItem {
        opacity: control.enabled ? 1.0 : 0.6
        width: height
        imagePath: "widgets/button"
        prefix: "shadow"

        PlasmaCore.FrameSvgItem {
            id: button
            imagePath: "widgets/button"
            prefix: "normal"
            anchors.fill: parent
        }
    }

    groove: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/slider"
        prefix: "groove"
        implicitHeight: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height
        implicitWidth: height * 2

        PlasmaCore.FrameSvgItem {
            id: highlight
            imagePath: "widgets/slider"
            prefix: "groove-highlight"
            anchors.fill: parent

            opacity: checked ? 1 : 0
            Behavior on opacity {
                PropertyAnimation { duration: PlasmaCore.Units.shortDuration * 2 }
            }
        }
    }
}
