// SPDX-FileCopyrightText: 2019 Carl-Lucien Schwan <carl@carlschwan.eu>
// SPDX-FileCopyrightText: 2022 Felipe Kinoshita <kinofhek@gmail.com>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * This is a standard textfield following KDE HIG. Using Ctrl+F as focus
 * sequence and "Search..." as placeholder text.
 *
 * Example usage for the search field component:
 * @code
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 *
 * PlasmaExtras.SearchField {
 *     id: searchField
 *     onAccepted: console.log("Search text is " + searchField.text)
 * }
 * @endcode
 *
 * @inherit org::plasmas::extras::ActionTextField
 * @since 5.93
 */
PlasmaExtras.ActionTextField {
    id: root

    property int _iconWidth: (activeFocus || root.text.length > 0 ? 0 : searchIcon.width)

    // padding to accommodate search icon nicely
    leftPadding: if (root.mirrored) {
        return _rightActionsRow.width + (__hasBackgroundAndMargins ? background.margins.left : 0);
    } else {
        return _iconWidth + _leftActionsRow.width + (__hasBackgroundAndMargins ? background.margins.left : 0);
    }
    rightPadding: if (root.mirrored) {
        return _iconWidth + _leftActionsRow.width + (__hasBackgroundAndMargins ? background.margins.right : 0);
    } else {
        return _rightActionsRow.width + (__hasBackgroundAndMargins ? background.margins.right : 0);
    }

    PlasmaCore.IconItem {
        id: searchIcon
        opacity: parent.activeFocus || text.length > 0 ? 0 : 1
        anchors.left: parent.left
        anchors.leftMargin: PlasmaCore.Units.smallSpacing * 2
        anchors.bottom: parent.bottom
        anchors.bottomMargin: Math.round((parent.implicitHeight - implicitHeight) / 2 + (parent.bottomPadding - parent.topPadding) / 2)
        implicitHeight: PlasmaCore.Units.iconSizes.sizeForLabels
        implicitWidth: PlasmaCore.Units.iconSizes.sizeForLabels
        status: PlasmaCore.Svg.Inactive

        source: "search"

        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    placeholderText: i18nd("libplasma5", "Search…")

    Accessible.name: i18nd("libplasma5", "Search")
    Accessible.searchEdit: true

    focusSequence: "Ctrl+F"
    inputMethodHints: Qt.ImhNoPredictiveText
    rightActions: QQC2.Action {
        icon.name: root.mirrored ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
        enabled: root.text.length > 0
        onTriggered: root.text = ""
    }
}