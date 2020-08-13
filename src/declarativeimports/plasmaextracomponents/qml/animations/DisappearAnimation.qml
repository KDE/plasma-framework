// -*- coding: iso-8859-1 -*-
/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2

SequentialAnimation {
    id: disappearAnimation
    objectName: "disappearAnimation"

    property Item targetItem
    property int duration: PlasmaCore.Units.longDuration

    ParallelAnimation {
        OpacityAnimator {
            duration: disappearAnimation.duration
            from: 1.0
            to: 0
            target: disappearAnimation.targetItem
            easing.type: Easing.OutExpo
        }
        ScaleAnimator {
            target: disappearAnimation.targetItem
            from: 1.0
            to: 0.8
            duration: disappearAnimation.duration * 0.6
            easing.type: Easing.OutExpo
        }
    }

    ScriptAction {
        script: {
            targetItem.visible = false;
        }
    }
}
