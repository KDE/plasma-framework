/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.2

import QtQuick.Controls 2.15 as Controls
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

ColumnLayout {
    Controls.Label {
        Layout.maximumWidth: mainLayout.width
        wrapMode: Text.WordWrap
        text: "Hover over every rectangle so that the tooltip pops up. It should popup in the correct position"
    }

    RowLayout {
        id: mainLayout
        Rectangle {
            width: 300
            height: 100
            color: "red"

            PlasmaCore.ToolTipArea {
                width: 300
                height: 50

                mainText: "Title Number 1"
                subText: "subtext"
                icon: "plasma"
            }
        }

        Rectangle {
            width: 300
            height: 100
            color: "blue"

            PlasmaCore.ToolTipArea {
                width: 500
                height: 110

                mainText: "Title Number 2"
                subText: "This is some really really really long subtext. So lets write stores about the woods and the trees and how we're going hiking. Yaye!"
                icon: "configure"
            }
        }

        Rectangle {
            width: 300
            height: 100
            color: "green"

            PlasmaCore.ToolTipArea {
                width: 350
                height: 70

                mainText: "Wakka Wakka"
                subText: "It's time for Africa"
            }
        }
    }
}
