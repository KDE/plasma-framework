/*
 * Copyright 2013 Bhushan Shah <bhush94@gmail.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License or (at your option) version 3 or any later version
 * accepted by the membership of KDE e.V. (or its successor approved
 * by the membership of KDE e.V.), which shall act as a proxy
 * defined in Section 14 of version 3 of the license.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

import QtQuick 2.0

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.SvgItem {

    id: meterPointer

    //property alias angle: rotation.angle
    implicitHeight: forgroundSvg.height / 2
    implicitWidth: forgroundSvg.height * 0.05

    anchors.bottom: parent.bottom
    anchors.horizontalCenter: parent.horizontalCenter

    svg: meterSvg
    elementId: "pointer"
    smooth: true

    transform: Rotation {
        id: rotation
        angle: 0
        origin {
            x: forgroundSvg.width /2
            y: 0
        }
//         Behaviour on angle {
//             //????
//         }
    }

}