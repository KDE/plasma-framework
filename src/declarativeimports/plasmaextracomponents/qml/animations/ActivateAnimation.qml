// -*- coding: iso-8859-1 -*-
/*
    SPDX-FileCopyrightText: 2011 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.kirigami 2 as Kirigami

SequentialAnimation {
    id: activateAnimation
    objectName: "activateAnimation"

    property Item targetItem
    property int duration: Kirigami.Units.shortDuration

    // Fast scaling while we're animation == more FPS
    ScriptAction { script: targetItem.smooth = false }

    PressedAnimation { targetItem: activateAnimation.targetItem }
    ReleasedAnimation  { targetItem: activateAnimation.targetItem }

    ScriptAction { script: targetItem.smooth = true }
}
