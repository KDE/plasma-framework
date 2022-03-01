/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 2.15 as Controls
import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.FrameSvgItem {
    id: root
    imagePath: "widgets/background"
    state: PlasmaCore.Svg.Normal
    width: 600
    height: 800

    Column {
        anchors.centerIn: parent
        spacing: 4

        Controls.Button {
            text: "Switch Selected State"
            onClicked: root.state = (root.state == PlasmaCore.Svg.Selected ? PlasmaCore.Svg.Normal : PlasmaCore.Svg.Selected)
        }

        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                id: svg
                imagePath: "icons/phone"
                state: root.state
            }
        }

        PlasmaCore.IconItem {
            id: icon
            source: "phone"
            state: root.state
        }
    }
}
