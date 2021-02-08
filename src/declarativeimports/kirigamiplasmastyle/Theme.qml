/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma Singleton

import QtQuick 2.4
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.16 as Kirigami

Kirigami.BasicThemeDefinition {
    textColor: theme.textColor
    disabledTextColor: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.6)

    highlightColor: theme.highlightColor
    highlightedTextColor: theme.highlightedTextColor
    backgroundColor: theme.backgroundColor
    alternateBackgroundColor: Qt.darker(theme.backgroundColor, 1.05)

    focusColor: theme.highlightColor
    hoverColor: theme.highlightColor

    //TODO: don't make this invisible
    activeTextColor: theme.highlightColor
    linkColor: theme.linkColor
    visitedLinkColor: theme.visitedLinkColor
    negativeTextColor: theme.negativeTextColor
    neutralTextColor: theme.neutralTextColor
    positiveTextColor: theme.positiveTextColor

    buttonTextColor: theme.buttonTextColor
    buttonBackgroundColor: theme.buttonBackgroundColor
    buttonAlternateBackgroundColor: Qt.darker(theme.buttonBackgroundColor, 1.05)
    buttonHoverColor: theme.buttonHoverColor
    buttonFocusColor: theme.buttonFocusColor

    viewTextColor: theme.viewTextColor
    viewBackgroundColor: theme.viewBackgroundColor
    viewAlternateBackgroundColor: Qt.darker(theme.viewBackgroundColor, 1.05)
    viewHoverColor: theme.viewHoverColor
    viewFocusColor: theme.viewFocusColor

    selectionTextColor: theme.highlightedTextColor
    selectionBackgroundColor: theme.highlightColor
    selectionAlternateBackgroundColor: Qt.darker(theme.highlightColor, 1.05)
    selectionHoverColor: theme.buttonHoverColor
    selectionFocusColor: theme.buttonFocusColor

    tooltipTextColor: theme.complementaryTextColor
    tooltipBackgroundColor: theme.complementaryBackgroundColor
    tooltipAlternateBackgroundColor: Qt.darker(theme.complementaryBackgroundColor, 1.05)
    tooltipHoverColor: theme.complementaryHoverColor
    tooltipFocusColor: theme.complementaryFocusColor

    complementaryTextColor: theme.complementaryTextColor
    complementaryBackgroundColor: theme.complementaryBackgroundColor
    complementaryAlternateBackgroundColor: Qt.darker(theme.complementaryBackgroundColor, 1.05)
    complementaryHoverColor: theme.complementaryHoverColor
    complementaryFocusColor: theme.complementaryFocusColor

    headerTextColor: theme.headerTextColor
    headerBackgroundColor: theme.headerBackgroundColor
    headerAlternateBackgroundColor: Qt.darker(theme.headerBackgroundColor, 1.05)
    headerHoverColor: theme.headerHoverColor
    headerFocusColor: theme.headerFocusColor

    defaultFont: theme.defaultFont
    smallFont: theme.smallestFont

    onSync: {
        object.PlasmaCore.ColorScope.inherit = false

        switch (object.Kirigami.Theme.colorSet) {
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
        case Kirigami.Theme.Header:
            object.PlasmaCore.ColorScope.colorGroup = PlasmaCore.Theme.HeaderColorGroup;
            break;
        }
    }
}
