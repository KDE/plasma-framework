/*
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.6
import org.kde.plasma.core 2.0 as PlasmaCore

Item {
    id: root
    required property bool hovered
    required property bool pressed
    required property bool checked
    required property bool focused

    property real leftMargin: surfaceHover.margins.left
    property real topMargin: surfaceHover.margins.top
    property real rightMargin: surfaceHover.margins.right
    property real bottomMargin: surfaceHover.margins.bottom
    property string usedPrefix: surfaceHover.usedPrefix

    ButtonShadow {
        anchors.fill: parent
        showShadow: !(root.checked || root.pressed) && root.usedPrefix === "normal"
    }

    ButtonFocus {
        anchors.fill: parent
        showFocus: root.focused && !root.pressed
        flat: true
    }

    // TODO: Maybe add a way to customize the look of normal state flat buttons with "toolbutton-normal"? 
    // TODO: Maybe add a way to customize the background of focused flat buttons with "toolbutton-focus-background"? 
    // TODO KF6: "flat" would be a more logical name than "toolbutton" since toolbuttons can be non-flat.
    PlasmaCore.FrameSvgItem {
        id: surfaceHover
        anchors.fill: parent
        imagePath: "widgets/button"
        /* TODO KF6: making "toolbutton-hover" margins work like "hover"
         * and using "hover" as a fallback would make more sense.
         * If that is done, make ButtonHover handle flat button hover effects.
         */
        // The fallback is "normal" to match PC2 behavior. Some 3rd party themes depend on this.
        prefix: ["toolbutton-hover", "normal"]
        visible: root.hovered
    }

    PlasmaCore.FrameSvgItem {
        id: surfacePressed
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: ["toolbutton-pressed", "pressed"]
        opacity: root.checked || root.pressed ? 1 : 0
        Behavior on opacity { OpacityAnimator { duration: PlasmaCore.Units.shortDuration; easing.type: Easing.OutQuad } }
    }
}
