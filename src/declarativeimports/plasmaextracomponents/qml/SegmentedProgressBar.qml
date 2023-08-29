/* SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 * SPDX-FileCopyrightText: 2023 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.15
import QtQuick.Templates 2.15 as T
import org.kde.ksvg 1.0 as KSvg
import org.kde.kirigami 2 as Kirigami

T.ProgressBar {
    id: control

    property int segments: 1

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: false

    KSvg.Svg {
        id: barSvg
        imagePath: "widgets/bar_meter_horizontal"
        // FIXME: see commit ccb1a4ca9f870050091166b5d2e65ecc038cf74d
        colorSet: control.Kirigami.Theme.colorSet
    }

    spacing: barSvg.hasElement("bar-active-separator") ?
        barSvg.elementSize("bar-active-separator").width : 0

    contentItem: Row {
        spacing: control.spacing
        LayoutMirroring.enabled: control.mirrored
        LayoutMirroring.childrenInherit: true
        property real indeterminateProgress: 0

        Repeater {
            model: control.segments
            delegate: KSvg.FrameSvgItem {
                imagePath: "widgets/bar_meter_horizontal"
                prefix: "bar-active"
                enabledBorders: {
                    let borders = KSvg.FrameSvgItem.TopBorder | KSvg.FrameSvgItem.BottomBorder
                    if ((!control.mirrored && Positioner.isFirstItem) || (control.mirrored && Positioner.isLastItem)) {
                        borders |= KSvg.FrameSvgItem.LeftBorder
                    }
                    if ((control.mirrored && Positioner.isFirstItem) || (!control.mirrored && Positioner.isLastItem)) {
                        borders |= KSvg.FrameSvgItem.RightBorder
                    }
                    return borders
                }

                anchors.verticalCenter: parent.verticalCenter

                // unlike Slider, this width is allowed to be less than its minimum (margins) size, in which case it would not render at all.
                width: control.segments > 0 ?
                    Math.round((Kirigami.Units.gridUnit * 8 - control.spacing * (control.segments - 1)) / control.segments) : 0
                implicitWidth: fixedMargins.left + fixedMargins.right
                implicitHeight: barSvg.hasElement("hint-bar-size") ?
                    barSvg.elementSize("hint-bar-size").height
                    : fixedMargins.top + fixedMargins.bottom

                visible: width >= implicitWidth

                KSvg.SvgItem {
                    svg: barSvg
                    elementId: "bar-active-separator"
                    visible: parent.enabledBorders !== KSvg.FrameSvgItem.AllBorders && !parent.Positioner.isLastItem
                    width: naturalSize.width
                    anchors.left: parent.right
                    height: parent.height
                }

                opacity: (control.indeterminate && Math.ceil(parent.indeterminateProgress) === Positioner.index + 1)
                    || (!control.indeterminate && Math.floor(control.position * control.segments) >= Positioner.index + 1)
            }
        }
        NumberAnimation on indeterminateProgress {
            loops: Animation.Infinite
            running: control.indeterminate && control.segments > 0 && control.contentItem.visible
            duration: Kirigami.Units.humanMoment
            from: 0
            to: 1
        }
    }

    background: Row {
        spacing: control.spacing
        LayoutMirroring.enabled: control.mirrored
        LayoutMirroring.childrenInherit: true

        Repeater {
            model: control.segments
            delegate: KSvg.FrameSvgItem {
                imagePath: "widgets/bar_meter_horizontal"
                prefix: "bar-inactive"
                enabledBorders: {
                    let borders = KSvg.FrameSvgItem.TopBorder | KSvg.FrameSvgItem.BottomBorder
                    if ((!control.mirrored && Positioner.isFirstItem) || (control.mirrored && Positioner.isLastItem)) {
                        borders |= KSvg.FrameSvgItem.LeftBorder
                    }
                    if ((control.mirrored && Positioner.isFirstItem) || (!control.mirrored && Positioner.isLastItem)) {
                        borders |= KSvg.FrameSvgItem.RightBorder
                    }
                    return borders
                }

                anchors.verticalCenter: parent.verticalCenter

                // unlike Slider, this width is allowed to be less than its minimum (margins) size, in which case it would not render at all.
                width: control.segments > 0 ?
                    Math.round((Kirigami.Units.gridUnit * 8 - control.spacing * (control.segments - 1)) / control.segments) : 0
                implicitWidth: fixedMargins.left + fixedMargins.right
                implicitHeight: barSvg.hasElement("hint-bar-size") ?
                    barSvg.elementSize("hint-bar-size").height
                    : fixedMargins.top + fixedMargins.bottom

                visible: width >= implicitWidth

                KSvg.SvgItem {
                    svg: barSvg
                    elementId: "bar-inactive-separator"
                    visible: parent.enabledBorders !== KSvg.FrameSvgItem.AllBorders && !parent.Positioner.isLastItem
                    width: naturalSize.width
                    anchors.left: parent.right
                    height: parent.height
                }
            }
        }
    }
}
