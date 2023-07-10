/*
    SPDX-FileCopyrightText: 2021 Arjen Hiemstra <ahiemstra@heimr.nl>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15

import org.kde.ksvg 1.0 as KSvg

Rectangle {
    color: "white"
    width: 600
    height: 600

    GridLayout {
        anchors.fill: parent
        columns: 3

        Repeater {
            model: [
                "shadow-topleft",
                "shadow-top",
                "shadow-topright",
                "shadow-left",
                "shadow-middle",
                "shadow-right",
                "shadow-bottomleft",
                "shadow-bottom",
                "shadow-bottomright"
            ]

            KSvg.SvgItem {
                elementId: modelData

                svg: KSvg.Svg {
                    imagePath: "dialogs/background"
                }
            }
        }
    }
}

