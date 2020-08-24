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
        /* implicitWidth and implicitHeight won't work unless they come
         * from a child of the contentItem. No idea why.
         */
        implicitWidth: PlasmaCore.Units.gridUnit * 2
        implicitHeight: implicitWidth

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
                duration: 2000
                loops: Animation.Infinite
            }
        }
    }

    state: control.running ? "running" : "hidden"

    states: [
        State {
            name: "hidden"
            PropertyChanges {
                target: contentItem
                opacity: 0
            }
        },
        State {
            name: "running"
            PropertyChanges {
                target: contentItem
                opacity: 1
            }
        }
    ]

    transitions: [
        Transition {
            from: "*"
            to: "hidden"
            SequentialAnimation {
                OpacityAnimator {
                    duration: PlasmaCore.Units.shortDuration
                    easing.type: Easing.OutQuad
                }
                PropertyAction {
                    target: contentItem
                    property: "visible"
                    value: false
                }
            }
            PropertyAction {
                target: rotationAnimator
                property: "running"
                value: false
            }
        },
        Transition {
            from: "*"
            to: "running"
            PropertyAction {
                target: rotationAnimator
                property: "running"
                value: true
            }
            SequentialAnimation {
                PropertyAction {
                    target: contentItem
                    property: "visible"
                    value: true
                }
                OpacityAnimator {
                    duration: PlasmaCore.Units.shortDuration
                    easing.type: Easing.OutQuad
                }
            }
        }
    ]
}
