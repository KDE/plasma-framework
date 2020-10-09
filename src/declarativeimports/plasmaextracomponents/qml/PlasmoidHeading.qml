/*
    SPDX-FileCopyrightText: 2020 Niccolò Venerandi <niccolo@venerandi.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.kde.plasma.core 2.0 as PlasmaCore
import QtQuick.Templates 2.12 as T

 /**
  * Item to be used as a header or footer in plasmoids
  * 
  * @inherit QtQuick.Templates.Frame
  */
 T.Frame {
    id: control
     /**
      * Possible positions of the heading element
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
     */
    property int location: PlasmoidHeading.Location.Header

    Layout.fillWidth: true
    bottomPadding: location == PlasmoidHeading.Location.Footer ? 0 : headingSvg.fixedMargins.top
    topPadding: location == PlasmoidHeading.Location.Footer ? headingSvg.fixedMargins.bottom : 0

    implicitHeight: contentHeight + topPadding + bottomPadding
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    leftInset: -headingSvg.fixedMargins.left
    rightInset: -headingSvg.fixedMargins.right
    topInset: location == PlasmoidHeading.Location.Footer ? 0 : -headingSvg.fixedMargins.top
    bottomInset: location == PlasmoidHeading.Location.Footer ? -headingSvg.fixedMargins.bottom : 0

    PlasmaCore.ColorScope.colorGroup: location == PlasmoidHeading.Location.Header ? PlasmaCore.Theme.HeaderColorGroup : PlasmaCore.Theme.WindowColorGroup

    background: PlasmaCore.FrameSvgItem {
        id: headingSvg
        visible: fromCurrentTheme
        imagePath: "widgets/plasmoidheading"
        prefix: location == PlasmoidHeading.Location.Header? 'header' : 'footer'

        colorGroup: control.PlasmaCore.ColorScope.colorGroup
        PlasmaCore.ColorScope.inherit: false

        enabledBorders: {
            var borders = 0
            borders |= PlasmaCore.FrameSvg.LeftBorder
            borders |= PlasmaCore.FrameSvg.RightBorder
            if (plasmoid.location !== PlasmaCore.Types.TopEdge || location != PlasmoidHeading.Location.Header) {
                borders |= PlasmaCore.FrameSvg.TopBorder
            }
            if (plasmoid.location !== PlasmaCore.Types.BottomEdge || location != PlasmoidHeading.Location.Footer) {
                borders |= PlasmaCore.FrameSvg.BottomBorder
            }
            return borders
        }
    }
 }
