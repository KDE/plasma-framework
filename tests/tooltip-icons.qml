/*
    SPDX-FileCopyrightText: 2014 Bhushan Shah <bhush94@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.20 as Kirigami

Row {
    height: 200
    width: 400
    Kirigami.Icon {
        source: "plasma"
        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: "Tooltip Title"
            subText: "Some explanation."
            icon: "plasma"
        }
    }

    Kirigami.Icon {
        source: "ark"
        PlasmaCore.ToolTipArea {
            anchors.fill: parent
            mainText: "Tooltip title"
            subText: "No icon!"
        }
    }
}

