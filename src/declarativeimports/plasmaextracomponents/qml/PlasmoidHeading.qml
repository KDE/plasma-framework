/*
    SPDX-FileCopyrightText: 2020 Niccolò Venerandi <niccolo@venerandi.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.kde.plasma.core 2.0 as PlasmaCore
import QtQuick.Templates 2.12 as T
import "private" as Private

/**
 * Item to be used as a header or footer in plasmoids
 *
 * @inherit QtQuick.Templates.ToolBar
 */
T.ToolBar {
    id: control
    /**
     * Possible positions of the heading element
     *
     * @deprecated since 5.85, use position from QtQuick.Templates.ToolBar and
     *  its ToolBar.Header & ToolBar.Footer enum values instead
     */
    enum Location {
        /**
         * Indicates that it's used as a header of the plasmoid, touching the top border
         */
        Header,
        /**
         * Indicates that it's used as a footer of the plasmoid, touching the bottom border
         */
        Footer
    }

    /**
     * location: int
     *
     * Indicates the position of the heading. The default is PlasmoidHeading.Location.Header.
     *
     * @deprecated since 5.85, use position from QtQuick.Templates.ToolBar instead
     */
    property alias location: control.position

    Layout.fillWidth: true
    bottomPadding: position === T.ToolBar.Footer ? 0 : -backgroundMetrics.getMargin("bottom")
    topPadding: position === T.ToolBar.Footer ? -backgroundMetrics.getMargin("top") : 0

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    leftInset: backgroundMetrics.getMargin("left")
    rightInset: backgroundMetrics.getMargin("right")
    topInset: position === T.ToolBar.Footer ? 0 : backgroundMetrics.getMargin("top")
    bottomInset: position === T.ToolBar.Footer ? backgroundMetrics.getMargin("bottom") : 0

    PlasmaCore.ColorScope.colorGroup: position === T.ToolBar.Header ? PlasmaCore.Theme.HeaderColorGroup : PlasmaCore.Theme.NormalColorGroup
    PlasmaCore.ColorScope.inherit: false

    background: PlasmaCore.FrameSvgItem {
        id: headingSvg
        // This graphics has to back with the dialog background, so it can be used if:
        // * both this and the dialog background are from the current theme
        // * both this and the dialog background are from fallback
        visible: fromCurrentTheme === backgroundSvg.fromCurrentTheme
        imagePath: "widgets/plasmoidheading"
        prefix: position === T.ToolBar.Header ? "header" : "footer"
        PlasmaCore.Svg {
            id: backgroundSvg
            imagePath: "dialogs/background"
        }

        colorGroup: control.PlasmaCore.ColorScope.colorGroup
        PlasmaCore.ColorScope.inherit: false

        enabledBorders: {
            let borders = PlasmaCore.FrameSvg.LeftBorder | PlasmaCore.FrameSvg.RightBorder;
            if (plasmoid.position !== PlasmaCore.Types.TopEdge || position !== T.ToolBar.Header) {
                borders |= PlasmaCore.FrameSvg.TopBorder;
            }
            if (plasmoid.position !== PlasmaCore.Types.BottomEdge || position !== T.ToolBar.Footer) {
                borders |= PlasmaCore.FrameSvg.BottomBorder;
            }
            return borders;
        }
        Private.BackgroundMetrics {
            id: backgroundMetrics
            function getMargin(margin) {
                if (!hasInset) {
                    return -headingSvg.fixedMargins[margin];
                } else {
                    return -backgroundMetrics.fixedMargins[margin] + backgroundMetrics.inset[margin]
                }
            }
        }
    }
}
