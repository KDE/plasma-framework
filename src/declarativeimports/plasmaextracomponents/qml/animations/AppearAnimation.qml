// -*- coding: iso-8859-1 -*-
/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2

SequentialAnimation {
    id: appearAnimation
    objectName: "appearAnimation"

    property Item targetItem
    property int duration: PlasmaCore.Units.longDuration

    // Animators run on the render thread so they kick in slightly delayed
    // so explicitly set the item's opacity to 0 before starting the animation
    ScriptAction {
        script: {
            targetItem.opacity = 0
        }
    }

    ParallelAnimation {
        OpacityAnimator {
            target: targetItem
            from: 0
            to: 1.0
            duration: appearAnimation.duration
            easing.type: Easing.InExpo
        }
        ScaleAnimator {
            target: targetItem
            from: 0.8
            to: 1.0
            duration: appearAnimation.duration
            easing.type: Easing.InExpo
        }
    }
}
