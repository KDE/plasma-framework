/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls.Styles 1.2 as QtQuickControlStyle
import QtQuick.Controls.Private 1.0 as QtQuickControlsPrivate
import QtQuick.Controls 1.2
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import "." as Style

/**
 * \internal
 */
QtQuickControlStyle.MenuBarStyle {
    id: styleRoot

    function formatMnemonic(text, underline) {
        return underline ? QtQuickControlsPrivate.StyleHelpers.stylizeMnemonics(text) : StyleHelpers.removeMnemonics(text)
    }

    background: Item {
        Accessible.role: Accessible.MenuBar
    }

    itemDelegate: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/tabbar"
        prefix: styleData.open || styleData.selected ? "north-active-tab" : ""
        property string plainText: QtQuickControlsPrivate.StyleHelpers.removeMnemonics(styleData.text)
        implicitWidth: text.implicitWidth + PlasmaCore.Units.smallSpacing * 3
        implicitHeight: text.implicitHeight + PlasmaCore.Units.smallSpacing * 2
        enabled: styleData.enabled
        colorGroup: PlasmaCore.ColorScope.colorGroup

        Accessible.role: Accessible.MenuItem
        Accessible.name: plainText

        PlasmaComponents.Label {
            id: text
            text: formatMnemonic(styleData.text, styleData.underlineMnemonic)
            anchors.centerIn: parent
        }
    }

    menuStyle: Style.MenuStyle { }
}
