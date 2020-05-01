/*
 *   Copyright 2020 Niccolò Venerandi <niccolo@venerandi.org>
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
    bottomPadding: location == PlasmoidHeading.Location.Footer ? 0 : headingSvg.margins.top
    topPadding: location == PlasmoidHeading.Location.Footer ? headingSvg.margins.bottom : 0

    implicitHeight: contentHeight + topPadding + bottomPadding
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    leftInset: -headingSvg.fixedMargins.left
    rightInset: -headingSvg.fixedMargins.right
    topInset: location == PlasmoidHeading.Location.Footer ? 0 : -headingSvg.margins.top
    bottomInset: location == PlasmoidHeading.Location.Footer ? -headingSvg.fixedMargins.bottom : 0

    background: PlasmaCore.FrameSvgItem {
        id: headingSvg
        visible: fromCurrentTheme
        imagePath: "widgets/plasmoidheading"
        prefix: location == PlasmoidHeading.Location.Header? 'header' : 'footer'
    }
 }
