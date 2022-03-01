/*
    SPDX-FileCopyrightText: 2020 Carson Black <uhhadd@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import org.kde.kirigami 2.12 as Kirigami
import org.kde.plasma.core 2.0 as PlasmaCore

Kirigami.BasicListItem {
    id: __listItem

    leftPadding: __background.margins.left
    rightPadding: __background.margins.right
    topPadding: __background.margins.top
    bottomPadding: __background.margins.bottom

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)

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
