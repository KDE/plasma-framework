/*
 *   Copyright 2020 Carson Black <uhhadd@gmail.com>
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
import org.kde.kirigami 2.12 as Kirigami
import org.kde.plasma.core 2.0 as PlasmaCore

Kirigami.AbstractListItem {
    id: __listItem

    /**
     * content: list<Object>
     *
     * The content of this ListItem.
     */
    default property alias content: __innerItem.data

    /**
     * separatorVisible: bool
     *
     * Whether this ListItem displays a separator.
     */
    property bool separatorVisible: true

    /**
     * sectionDelegate: bool
     *
     * Whether this ListItem renders as a section header.
     */
    property bool sectionDelegate: true

    /**
     * containsMouse: bool
     *
     * Whether this ListItem contains the mouse.
     *
     * Alias of Control.hovered.
     */
    readonly property bool containsMouse: hovered

    leftPadding: __background.margins.left
    rightPadding: __background.margins.right
    topPadding: __background.margins.top
    bottomPadding: __background.margins.bottom

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset, implicitContentWidth + leftPadding + rightPadding)

    // TODO KF6: Make this behave more like the normal Control default.
    // Behaving this way for backwards compatibility reasons.
    contentItem: Item {
        id: __innerItem
    }

    background: PlasmaCore.FrameSvgItem {
        id : __background
        imagePath: "widgets/listitem"
        prefix: (__listItem.sectionDelegate ? "section" :
                (__listItem.pressed || __listItem.checked) ? "pressed" : "normal")

        anchors.fill: parent
        visible: __listItem.ListView.view ? __listItem.ListView.view.highlight === null : true

        PlasmaCore.SvgItem {
            svg: PlasmaCore.Svg {
                imagePath: "widgets/listitem"
            }
            elementId: "separator"
            anchors {
                left: parent.left
                right: parent.right
                top: parent.top
            }
            height: naturalSize.height
            visible: __listItem.separatorVisible && (__listItem.sectionDelegate || (typeof(index) != "undefined" && index > 0 && !__listItem.checked && !__listItem.pressed))
        }
    }
}