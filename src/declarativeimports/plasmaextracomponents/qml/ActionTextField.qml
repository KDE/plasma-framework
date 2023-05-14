// SPDX-FileCopyrightText: 2019 Carl Schwan <carl@carlschwan.eu>
// SPDX-License-Identifier: LGPL-2.0-or-later

import QtQuick 2.15
import QtQuick.Controls 2.15 as QQC2
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami

/**
 * This is advanced textfield. It is recommended to use this class when there
 * is a need to create a create a textfield with action buttons (e.g a clear
 * action).
 *
 * For common pattern like, a search field or a password field, prefer using the
 * more specifig org::kde::extras::SearchField or org::kde::extras::PasswordField.
 *
 * Example usage for a search field:
 * @code
 * import QtQuick.Controls 2.15 as QQC2
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 *
 * PlasmaExtras.ActionTextField {
 *     id: searchField
 *
 *     placeholderText: "Search…"
 *
 *     focusSequence: StandardKey.Find
 *
 *     rightActions: [
 *         QQC2.Action {
 *             icon.name: "edit-clear"
 *             enabled: searchField.text !== ""
 *             onTriggered: {
 *                 searchField.clear()
 *                 searchField.accepted()
 *             }
 *         }
 *     ]
 *
 *     onAccepted: console.log("Search text is " + searchField.text)
 * }
 * @endcode
 *
 * @inherit QtQuick.Controls.TextField
 * @since 5.93
 * @author Carl Schwan <carl@carlschwan.eu>
 */
PlasmaComponents3.TextField {
    id: root

    /**
     * This property holds a shortcut sequence that will focus the text field.
     *
     * @property QtQuick.Shortcut.sequence focusSequence
     * @since 5.93
     */
    property alias focusSequence: focusShortcut.sequence

    /**
     * This property holds a list of actions that will be displayed on the left side of the text field.
     *
     * By default this list is empty.
     *
     * @since 5.93
     */
    property list<QQC2.Action> leftActions

    /**
     * This property holds a list of actions that will be displayed on the right side of the text field.
     *
     * By default this list is empty.
     *
     * @since 5.93
     */
    property list<QQC2.Action> rightActions

    property alias _leftActionsRow: leftActionsRow
    property alias _rightActionsRow: rightActionsRow

    hoverEnabled: true

    topPadding: __hasBackgroundAndMargins ? background.margins.top : 0
    bottomPadding: __hasBackgroundAndMargins ? background.margins.bottom : 0

    leftPadding: if (root.effectiveHorizontalAlignment === TextInput.AlignRight) {
        return _rightActionsRow.width + (__hasBackgroundAndMargins ? background.margins.left : 0);
    } else {
        return _leftActionsRow.width + (__hasBackgroundAndMargins ? background.margins.left : 0);
    }
    rightPadding: if (root.effectiveHorizontalAlignment === TextInput.AlignRight) {
        return _leftActionsRow.width + (__hasBackgroundAndMargins ? background.margins.right : 0);
    } else {
        return _rightActionsRow.width + (__hasBackgroundAndMargins ? background.margins.right : 0);
    }

    Behavior on leftPadding {
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    Behavior on rightPadding {
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.InOutQuad
        }
    }

    Shortcut {
        id: focusShortcut
        onActivated: {
            root.forceActiveFocus(Qt.ShortcutFocusReason)
            root.selectAll()
        }

        // here to make it private
        component ActionIcon: PlasmaCore.IconItem {
            implicitWidth: PlasmaCore.Units.iconSizes.small
            implicitHeight: PlasmaCore.Units.iconSizes.small

            anchors.verticalCenter: parent.verticalCenter

            source: modelData.icon.name.length > 0 ? modelData.icon.name : modelData.icon.source
            visible: modelData.enabled
            MouseArea {
                onClicked: modelData.trigger()
                cursorShape: Qt.ArrowCursor
                anchors.fill: parent
            }
        }
    }

    PlasmaComponents3.ToolTip.visible: focusShortcut.nativeText.length > 0 && root.text.length === 0 && !rightActionsRow.hovered && !leftActionsRow.hovered && hovered
    PlasmaComponents3.ToolTip.text: focusShortcut.nativeText
    PlasmaComponents3.ToolTip.delay: Kirigami.Settings.tabletMode ? Qt.styleHints.mousePressAndHoldInterval : Kirigami.Units.toolTipDelay

    Row {
        id: leftActionsRow
        padding: visible ? PlasmaCore.Units.smallSpacing : 0
        LayoutMirroring.enabled: root.effectiveHorizontalAlignment === TextInput.AlignRight
        anchors.left: parent.left
        anchors.leftMargin: PlasmaCore.Units.smallSpacing
        anchors.verticalCenter: parent.verticalCenter
        height: root.implicitHeight - 2 * PlasmaCore.Units.smallSpacing
        visible: root.leftActions.length > 0
        Repeater {
            model: root.leftActions
            ActionIcon { }
        }
    }

    Row {
        id: rightActionsRow
        padding: visible ? PlasmaCore.Units.smallSpacing : 0
        layoutDirection: Qt.RightToLeft
        LayoutMirroring.enabled: root.effectiveHorizontalAlignment === TextInput.AlignRight
        anchors.right: parent.right
        anchors.rightMargin: PlasmaCore.Units.smallSpacing
        anchors.verticalCenter: parent.verticalCenter
        height: root.implicitHeight - 2 * PlasmaCore.Units.smallSpacing
        visible: root.rightActions.length > 0
        Repeater {
            model: root.rightActions
            ActionIcon { }
        }
    }
}
