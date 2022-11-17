/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.12
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.FrameSvgItem {
    anchors {
        fill: parent
        leftMargin: -margins.left
        topMargin: -margins.top
        rightMargin: -margins.right
        bottomMargin: -margins.bottom
    }

    property bool showFocus: false
    property bool flat: false

    imagePath: "widgets/button"
    prefix: flat ? ["toolbutton-focus", "focus"] : "focus"

    opacity: showFocus ? 1 : 0
    Behavior on opacity { OpacityAnimator { duration: PlasmaCore.Units.shortDuration; easing.type: Easing.OutQuad } }
}
