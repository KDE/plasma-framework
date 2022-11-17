/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.12
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.FrameSvgItem {
    property bool showHover: false

    anchors {
        fill: parent
        leftMargin: -margins.left
        topMargin: -margins.top
        rightMargin: -margins.right
        bottomMargin: -margins.bottom
    }
    imagePath: "widgets/button"
    prefix: "hover"

    visible: showHover ? 1 : 0
}
