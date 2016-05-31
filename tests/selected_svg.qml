/*
 *   Copyright 2016 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.2
import QtQuick.Controls 1.1 as Controls
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
