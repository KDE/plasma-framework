/* SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {
    id: root
    property string focusFrameElement: ""
    property string focusElement: ""
    property string hoverElement: ""
    readonly property bool hasFocusFrame: focusFrameElement && root.svg && root.svg.hasElement(focusFrameElement)
    // Use this when setting the elementId to use a focus frame
    readonly property string effectiveFocusFrameElement: hasFocusFrame ? focusFrameElement : focusElement
    // Sometimes Behavior animations and Animators can cause problems when trying to keep animations in sync
    property bool enableBehaviorOpacityAnimator: true
    implicitWidth: root.naturalSize.width
    implicitHeight: root.naturalSize.height
    visible: opacity > 0
    Behavior on opacity {
        enabled: root.enableBehaviorOpacityAnimator
        OpacityAnimator {
            duration: PlasmaCore.Units.shortDuration
            easing.type: Easing.OutCubic
        }
    }
}
