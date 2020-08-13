/*
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
        visible: control.running
        svg: PlasmaCore.Svg {
            imagePath: "widgets/busywidget"
            colorGroup: PlasmaCore.ColorScope.colorGroup
        }
        elementId: "busywidget"

        implicitWidth:  units.gridUnit * 2
        implicitHeight: units.gridUnit * 2

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
            duration: 2000
            running: control.running && indicatorItem.visible && indicatorItem.opacity > 0;
            loops: Animation.Infinite
        }
    }
}
