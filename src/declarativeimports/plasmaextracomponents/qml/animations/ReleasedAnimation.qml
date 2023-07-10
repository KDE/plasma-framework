// -*- coding: iso-8859-1 -*-
/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.kirigami 2.20 as Kirigami

SequentialAnimation {
    id: releasedAnimation
    objectName: "releasedAnimation"

    property Item targetItem
    property int duration: Kirigami.Units.shortDuration

    // Fast scaling while we're animation == more FPS
    ScriptAction { script: targetItem.smooth = false }

    ParallelAnimation {
        PropertyAnimation {
            target: targetItem
            properties: "opacity"
            from: 0.8; to: 1.0
            duration: releasedAnimation.duration;
            easing.type: Easing.InExpo;
        }
        PropertyAnimation {
            target: targetItem
            properties: "scale"
            from: 0.95; to: 1.0
            duration: releasedAnimation.duration;
            easing.type: Easing.InExpo;
        }
    }

    ScriptAction { script: targetItem.smooth = true }
}
