/*
 *   Copyright (C) 2010 by Anselmo Lacerda Silveira de Melo <anselmolsm@gmail.com>
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

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: root
    width: 200
    height: 100

    //API
    property bool autohide: false
    //### get font from theme when available
    //### deal with QFont stuff in a proper way
    property string font: "Arial"
    property string color: theme.textColor
    property int duration: 3000

    /*### actually the FlashingLabel in C++ doesn't have
    this as part of its API, either text or pixmap is
    set only when flash is called.*/
    property string text: ""

    //###
    PlasmaCore.Theme {
        id: theme
    }

    Text {
        id: label
        text: root.text
        color: root.color
        font.family: root.font
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        anchors.verticalCenter: root.verticalCenter
        anchors.horizontalCenter: root.horizontalCenter
    }

    PropertyAnimation {
        id: fade
        target: label;
        property: "opacity";
        duration: root.duration;
        to: 0
    }

    function flash(text, duration, alignment) {
        if (text)
            root.text = text
        if (duration)
            root.duration = duration
        if (alignment)
            label.horizontalAlignment = alignment
        fade.start()
    }

    //###TODO: flash pixmap
}
