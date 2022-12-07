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
 * sequence and "Search…" as placeholder text.
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

    property int _iconWidth: (activeFocus || root.text.length > 0 ? 0 : searchIcon.width + searchIcon.anchors.leftMargin)

    // padding to accommodate search icon nicely
    leftPadding: if (root.effectiveHorizontalAlignment === TextInput.AlignRight) {
        return (_rightActionsRow.visible ? _rightActionsRow.width : 0) + (__hasBackgroundAndMargins ? background.margins.left : 0);
    } else {
        return _iconWidth + (_leftActionsRow.visible ? _leftActionsRow.width : 0) + (__hasBackgroundAndMargins ? background.margins.left : 0);
    }
    rightPadding: if (root.effectiveHorizontalAlignment === TextInput.AlignRight) {
        return _iconWidth + (_leftActionsRow.visible ? _leftActionsRow.width : 0) + (__hasBackgroundAndMargins ? background.margins.right : 0);
    } else {
        return (_rightActionsRow.visible ? _rightActionsRow.width : 0) + (__hasBackgroundAndMargins ? background.margins.right : 0);
    }

    PlasmaCore.IconItem {
        id: searchIcon
        opacity: root.activeFocus || text.length > 0 ? 0 : 1
        LayoutMirroring.enabled: root.effectiveHorizontalAlignment === TextInput.AlignRight
        anchors.left: root.left
        anchors.leftMargin: PlasmaCore.Units.smallSpacing * 2
        anchors.verticalCenter: root.verticalCenter
        anchors.verticalCenterOffset: Math.round((root.topPadding - root.bottomPadding) / 2)
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

    focusSequence: StandardKey.Find
    inputMethodHints: Qt.ImhNoPredictiveText
    rightActions: QQC2.Action {
        //ltr confusingly refers to the direction of the arrow in the icon, not the text direction which it should be used in
        icon.name: root.effectiveHorizontalAlignment === TextInput.AlignRight ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl"
        enabled: root.text.length > 0
        onTriggered: {
            root.clear();
        }
    }
}
