/*
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    property bool sectionDelegate: false

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

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)

    // TODO KF6: Make this behave more like the normal Control default.
    // Behaving this way for backwards compatibility reasons.
    contentItem: Item {
        id: __innerItem
    }

    background: PlasmaCore.FrameSvgItem {
        id: __background
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
