/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
//for Settings
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.2 as Kirigami

PlasmaCore.FrameSvgItem {
    id: background

    imagePath: "widgets/listitem"
    prefix: control.highlighted || control.pressed ? "pressed" : "normal"
    
    visible: control.ListView.view ? control.ListView.view.highlight === null : true

    PlasmaCore.FrameSvgItem {
        imagePath: "widgets/listitem"
        visible: !Kirigami.Settings.isMobile
        prefix: "hover"
        anchors.fill: parent
        opacity: control.hovered && !control.pressed ? 1 : 0
    }
}

