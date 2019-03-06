/*
 *   Copyright 2015 Marco Martin <mart@kde.org>
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

pragma Singleton

import QtQuick 2.4
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.2 as Kirigami

QtObject {
    property color textColor: theme.textColor
    property color disabledTextColor: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.6)

    property color highlightColor: theme.highlightColor
    property color highlightedTextColor: theme.highlightedTextColor
    property color backgroundColor: theme.backgroundColor
    property color alternateBackgroundColor: Qt.darker(theme.backgroundColor, 1.05)
    //TODO: don't make this invisible
    property color activeTextColor: theme.highlightColor
    property color linkColor: theme.linkColor
    property color visitedLinkColor: theme.visitedLinkColor
    property color negativeTextColor: theme.negativeTextColor
    property color neutralTextColor: theme.neutralTextColor
    property color positiveTextColor: theme.positiveTextColor

    property color buttonTextColor: theme.buttonTextColor
    property color buttonBackgroundColor: theme.buttonBackgroundColor
    property color buttonAlternateBackgroundColor: Qt.darker(theme.buttonBackgroundColor, 1.05)
    property color buttonHoverColor: theme.buttonHoverColor
    property color buttonFocusColor: theme.buttonFocusColor

    property color viewTextColor: theme.viewTextColor
    property color viewBackgroundColor: theme.viewBackgroundColor
    property color viewAlternateBackgroundColor: Qt.darker(theme.viewBackgroundColor, 1.05)
    property color viewHoverColor: theme.viewHoverColor
    property color viewFocusColor: theme.viewFocusColor

    property color selectionTextColor: theme.highlightedTextColor
    property color selectionBackgroundColor: theme.highlightColor
    property color selectionAlternateBackgroundColor: Qt.darker(theme.selectionBackgroundColor, 1.05)
    property color selectionHoverColor: theme.buttonHoverColor
    property color selectionFocusColor: theme.buttonFocusColor

    property color tooltipTextColor: theme.complementaryTextColor
    property color tooltipBackgroundColor: theme.complementaryBackgroundColor
    property color tooltipAlternateBackgroundColor: Qt.darker(theme.tooltipBackgroundColor, 1.05)
    property color tooltipHoverColor: theme.complementaryHoverColor
    property color tooltipFocusColor: theme.complementaryFocusColor

    property color complementaryTextColor: theme.complementaryTextColor
    property color complementaryBackgroundColor: theme.complementaryBackgroundColor
    property color complementaryAlternateBackgroundColor: Qt.darker(theme.complementaryBackgroundColor, 1.05)
    property color complementaryHoverColor: theme.complementaryHoverColor
    property color complementaryFocusColor: theme.complementaryFocusColor

    property variant defaultFont: theme.defaultFont

    function __propagateColorSet(object, context) {
        object.PlasmaCore.ColorScope.inherit = false;
        switch(context) {
        case Kirigami.Theme.Window:
            object.PlasmaCore.ColorScope.colorGroup = PlasmaCore.Theme.NormalColorGroup;
            break;
        case Kirigami.Theme.Button:
            object.PlasmaCore.ColorScope.colorGroup = PlasmaCore.Theme.ButtonColorGroup;
            break;
        case Kirigami.Theme.View:
            object.PlasmaCore.ColorScope.colorGroup = PlasmaCore.Theme.ViewColorGroup;
            break;
        case Kirigami.Theme.Selection:
            object.PlasmaCore.ColorScope.colorGroup = PlasmaCore.Theme.NormalColorGroup;
            break;
        case Kirigami.Theme.Tooltip:
            object.PlasmaCore.ColorScope.colorGroup = PlasmaCore.Theme.ComplementaryColorGroup;
            break;
        case Kirigami.Theme.Complementary:
            object.PlasmaCore.ColorScope.colorGroup = PlasmaCore.Theme.ComplementaryColorGroup;
            break;
        }
    }
    function __propagateTextColor(object, color) {}
    function __propagateBackgroundColor(object, color) {}
    function __propagatePrimaryColor(object, color) {}
    function __propagateAccentColor(object, color) {}
}
