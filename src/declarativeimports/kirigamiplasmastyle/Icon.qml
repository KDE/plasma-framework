/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.ksvg 1.0 as KSvg

PlasmaCore.IconItem {
    property bool selected: false
    property bool isMask: false
    //TODO: implement in libplasma
    property color color: "transparent"
    usesPlasmaTheme: false
    colorGroup: PlasmaCore.ColorScope.colorGroup
    onSelectedChanged: {
        if (selected) {
            status = KSvg.Svg.Selected;
        } else {
            status = KSvg.Svg.Normal;
        }
    }
}
