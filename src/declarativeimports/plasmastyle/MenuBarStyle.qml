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
import QtQuick.Controls.Styles 1.2 as QtQuickControlStyle
import QtQuick.Controls.Private 1.0 as QtQuickControlsPrivate
import QtQuick.Controls 1.2
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.core 2.0 as PlasmaCore
import "." as Style

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
        implicitWidth: text.implicitWidth + units.smallSpacing * 3
        implicitHeight: text.implicitHeight + units.smallSpacing * 2
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
