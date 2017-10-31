/*
 *   Copyright 2016 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.6
import QtQuick.Templates 2.0 as T
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

T.Dial {
    id: control

    implicitWidth: units.gridUnit * 5
    implicitHeight: implicitWidth
    hoverEnabled: true
    onPositionChanged: canvas.requestPaint()

    background:Canvas {
        id: canvas
        width: control.availableWidth
        height: control.availableHeight
        onPaint: {
            var ctx = getContext("2d");
            ctx.reset();

            var centreX = width / 2;
            var centreY = height / 2;

            ctx.globalAlpha = 0.3;
            ctx.beginPath();
            ctx.strokeStyle = theme.textColor;
            ctx.lineWidth=5;
            ctx.arc(centreX, centreY, width/2.4, 0, 2*Math.PI, false);
            ctx.stroke();
            ctx.globalAlpha = 1;

            ctx.beginPath();
            ctx.strokeStyle = theme.highlightColor;
            ctx.lineWidth=5;
            ctx.arc(centreX, centreY, width/2.4, 0.7*Math.PI, 1.6*Math.PI * control.position - 1.25*Math.PI, false);
            ctx.stroke();
        }
    }

    PlasmaCore.Svg {
        id: grooveSvg
        imagePath: "widgets/slider"
        colorGroup: PlasmaCore.ColorScope.colorGroup
    }
    handle: Item {
        x: (control.width/2) + Math.cos((-(control.angle-90)*Math.PI)/180) * (control.width/2-width/2) - width/2
        y: (control.height/2) + Math.sin(((control.angle-90)*Math.PI)/180) * (control.height/2-height/2) - height/2

        implicitHeight: Math.floor(units.gridUnit*1.6)
        implicitWidth: implicitHeight

        Private.RoundShadow {
            id: roundShadow
            anchors.fill: parent
            state: {
                if (control.pressed) {
                    return "hidden"
                } else if (control.hovered) {
                    return "hover"
                } else if (control.activeFocus) {
                    return "focus"
                } else {
                    return "shadow"
                }
            }
        }

        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                id: buttonSvg
                imagePath: "widgets/actionbutton"
            }
            elementId: control.pressed? "pressed" : "normal"
            width: Math.floor(parent.height/2) * 2
            height: width
            anchors.centerIn: parent
            Behavior on opacity {
                NumberAnimation { duration: units.longDuration }
            }
        }
    }
}
