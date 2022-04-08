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

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    /* PC2 BusyIndicator didn't have padding or margins and
     * BusyIndicator doesn't need padding since it has no background.
     * A Control containing a BusyIndicator can have padding instead
     * (e.g., a ToolBar, a Page or maybe a widget in a Plasma panel).
     */
    padding: 0

    contentItem: Item {
        id: baseItem
        // Don't want it to animate at all if the user has disabled animations
        property bool animationRunning: visible && (control.running || opacity > 0) && PlasmaCore.Units.longDuration > 1;

        /* implicitWidth and implicitHeight won't work unless they come
         * from a child of the contentItem. No idea why.
         */
        implicitWidth: PlasmaCore.Units.gridUnit * 2
        implicitHeight: implicitWidth

        visible: opacity > 0
        opacity: control.running ? 1.0 : 0.0
        Behavior on opacity {
            OpacityAnimator {
                duration: PlasmaCore.Units.shortDuration
                easing.type: Easing.OutQuad
            }
        }

        // sync all busy animations so they start at a common place in the rotation
        onAnimationRunningChanged: {
            if (animationRunning) {
                const date = new Date;
                const ms = date.valueOf();
                busyIndicatorSvgItem.rotation = ((ms % rotationAnimator.duration) / rotationAnimator.duration) * 360
                rotationAnimator.from = busyIndicatorSvgItem.rotation
                rotationAnimator.to = 360 + busyIndicatorSvgItem.rotation
            }
            rotationAnimator.running = animationRunning
        }

        PlasmaCore.SvgItem {
            id: busyIndicatorSvgItem

            /* Do not use `anchors.fill: parent` in here or else
             * the aspect ratio won't always be 1:1.
             */
            width: Math.min(parent.width, parent.height)
            height: width

            svg: PlasmaCore.Svg {
                imagePath: "widgets/busywidget"
                colorGroup: PlasmaCore.ColorScope.colorGroup
            }
            elementId: "busywidget"

            RotationAnimator on rotation {
                id: rotationAnimator
                from: 0
                to: 360
                // Not using a standard duration value because we don't want the
                // animation to spin faster or slower based on the user's animation
                // scaling preferences; it doesn't make sense in this context
                duration: 2000
                loops: Animation.Infinite
            }
        }
    }
}
