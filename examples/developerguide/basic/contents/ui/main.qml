/*
 *   Copyright 2015 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
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

import QtQuick 2.3
import QtQuick.Layouts 1.1
import QtQuick.Controls 1.0 as QtControls

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {

    // Initial size of the window in gridUnits
    width: units.gridUnit * 28
    height: units.gridUnit * 20

    // We use a ColumnLayout to position and size the individual items
    ColumnLayout {

        // Our ColumnLayout is fills the parent item with a bit of margin
        anchors {
            fill: parent
            margins: units.largeSpacing
        }

        spacing: units.gridUnit

        // A title on top
        PlasmaExtras.Title {
            text: i18n("Hello Plasma World!")
        }

        // The central area is a rectangle
        Rectangle {
            // The id is used to reference this item from the
            // button's onClicked function
            id: colorRect

            // It's supposed to grow in both direction
            Layout.fillWidth: true
            Layout.fillHeight: true
        }

        // A button to change the color to blue or green
        QtControls.Button {

            // The button is aligned to the right
            Layout.alignment: Qt.AlignRight

            // The button's label, ready for translations
            text: i18n("Change Color")

            onClicked: {
                // Simply switch colors of the rectangle around
                if (colorRect.color != "#b0c4de") {
                    colorRect.color = "#b0c4de"; // lightsteelblue
                } else {
                    colorRect.color = "lightgreen";
                }
                // This message will end up being printed to the terminal
                print("Color is now " + colorRect.color);
            }
        }
    }

    // Overlay everything with a decorative, large, translucent icon
    PlasmaCore.IconItem {

        // We use an anchor layout and dpi-corrected sizing
        width: units.iconSizes.large * 4
        height: width
        anchors {
            left: parent.left
            bottom: parent.bottom
        }

        source: "akregator"
        opacity: 0.1
    }
}
