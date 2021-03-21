/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Private 1.0
import QtQuick.Controls.Styles 1.2 as QtQuickControlStyle
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

//GroupBoxStyle is not exported
/**
 * \internal
 */
Style  {
    id: styleRoot

    property color textColor: PlasmaCore.ColorScope.textColor

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
            anchors.margins: PlasmaCore.Units.smallSpacing
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
