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
import QtQuick.Window 2.1
import QtQuick.Controls 1.2
import QtQuick.Controls.Private 1.0
import QtQuick.Controls.Styles 1.2 as QtQuickControlStyle
import org.kde.plasma.components 2.0 as PlasmaComponents

QtQuickControlStyle.MenuStyle {
    id: styleRoot

    __menuItemType: "menuitem"

    submenuOverlap: 0
    submenuPopupDelay: 0
    __maxPopupHeight: 0
    __leftLabelMargin: theme.mSize(theme.defaultFont).height * 2 //see RadioButtonStyle size

    frame: StyleItem {
        elementType: "menu"

        Rectangle {
            visible: anchors.margins > 0
            anchors {
                fill: parent
                margins: pixelMetric("menupanelwidth")
            }
            color: theme.backgroundColor
        }

        Accessible.role: Accessible.PopupMenu

        Component.onCompleted: {
            var menuHMargin = pixelMetric("menuhmargin")
            var menuVMargin = pixelMetric("menuvmargin")
            var menuPanelWidth = pixelMetric("menupanelwidth")
            styleRoot.padding.left = menuHMargin + menuPanelWidth
            styleRoot.padding.right = menuHMargin + menuPanelWidth
            styleRoot.padding.top = menuVMargin + menuPanelWidth
            styleRoot.padding.bottom = menuVMargin + menuPanelWidth
            styleRoot.submenuOverlap = 2 * menuPanelWidth
            styleRoot.submenuPopupDelay = styleHint("submenupopupdelay")
        }

        readonly property int desktopAvailableHeight: Screen.desktopAvailableHeight
        Binding {
            target: styleRoot
            property: "__maxPopupHeight"
            value: desktopAvailableHeight * 0.99
        }
    }

    __scrollerStyle: null

    itemDelegate.background: PlasmaComponents.Highlight {
        visible: styleData.selected && styleData.enabled
    }
    itemDelegate.label: PlasmaComponents.Label {
        text: formatMnemonic(styleData.text, styleData.underlineMnemonic)
    }
    itemDelegate.submenuIndicator: PlasmaComponents.Label {
        text: __mirrored ? "\u25c2" : "\u25b8" // BLACK LEFT/RIGHT-POINTING SMALL TRIANGLE
        style: styleData.selected ? Text.Normal : Text.Raised
        styleColor: Qt.lighter(color, 4)
    }

    itemDelegate.shortcut: PlasmaComponents.Label {
        text: styleData.shortcut
    }

    itemDelegate.checkmarkIndicator: Loader {
        sourceComponent: styleData.exclusive ? exclusiveCheckMark : nonExclusiveCheckMark
        Component {
            id: exclusiveCheckMark
            PlasmaComponents.RadioButton {
                width: parent.height
                height: parent.height
                checked: styleData.checked
            }
        }

        Component {
            id: nonExclusiveCheckMark
            PlasmaComponents.CheckBox {
                width: parent.height
                height: parent.height
                checked: styleData.checked
            }
        }
    }
}
