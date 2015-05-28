/*
 *   Copyright 2014 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */

import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Private 1.0
import QtQuick.Controls.Styles 1.2 as QtQuickControlStyle
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

//GroupBoxStyle is not exported
Style  {
    id: styleRoot

    property color textColor: theme.textColor

    property Component checkbox: PlasmaComponents.CheckBox {
        checked: control.checked
    }

    property Component panel: Item {
        anchors.fill: parent
        Loader {
            id: checkboxloader
            anchors.left: parent.left
            sourceComponent: control.checkable ? checkbox : null
            anchors.verticalCenter: label.verticalCenter
            width: item ? item.implicitWidth : 0
        }

        PlasmaComponents.Label {
            id: label
            anchors.top: parent.top
            anchors.left: checkboxloader.right
            anchors.margins: units.smallSpacing
            text: control.title
        }

        PlasmaCore.FrameSvgItem {
            id: frame
            anchors.fill: parent
            imagePath: "widgets/frame"
            prefix: "plain"
            visible: !control.flat
            colorGroup: PlasmaCore.ColorScope.colorGroup
            Component.onCompleted: {
                styleRoot.padding.left = frame.margins.left
                styleRoot.padding.top = frame.margins.top + label.height
                styleRoot.padding.right = frame.margins.right
                styleRoot.padding.bottom = frame.margins.bottom
            }
        }
    }
}
