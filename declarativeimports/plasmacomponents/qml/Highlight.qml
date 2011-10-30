/*
 *   Copyright 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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
import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: highlight

    property bool hover: false
    property bool pressed: false

    PlasmaCore.FrameSvgItem {
        id: background
        imagePath: "widgets/viewitem"
        prefix: {
            if (pressed)
                return hover ? "selected+hover" : "selected";

            if (hover)
                return "hover";

            return "normal";
        }

        anchors {
            fill: parent
            topMargin: -background.margins.top
            leftMargin: -background.margins.left
            bottomMargin: -background.margins.bottom
            rightMargin: -background.margins.right
        }
    }
}
