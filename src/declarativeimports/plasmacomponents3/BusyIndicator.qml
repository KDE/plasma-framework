/*
*   Copyright (C) 2014 Kai Uwe Broulik <kde@privat.broulik.de>
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
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.2
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.BusyIndicator {
    id: control

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    padding: units.smallSpacing

    contentItem: PlasmaCore.SvgItem {
        id: indicatorItem
        svg: PlasmaCore.Svg {
            imagePath: "widgets/busywidget"
            colorGroup: PlasmaCore.ColorScope.colorGroup
        }
        elementId: "busywidget"

        implicitWidth:  units.gridUnit * 2
        implicitHeight: width

        Connections {
            target: control
            onRunningChanged: {
                rotationAnimator.from = rotation
                rotationAnimator.to = rotation + 360
            }
        }

        RotationAnimator on rotation {
            id: rotationAnimator
            from: 0
            to: 360
            duration: 1500
            running: control.running && indicatorItem.visible && indicatorItem.opacity > 0;
            loops: Animation.Infinite
        }
    }
}
