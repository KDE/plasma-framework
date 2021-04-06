/*
 *  SPDX-FileCopyrightText: 2020 Nate Graham <nate@kde.org>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.0
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12 as QQC2

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.plasma.extras 2.0 as PlasmaExtras

/**
 * A placeholder message indicating that a list view is empty. The message
 * comprises a label with lightened text, an optional icon above the text, and
 * an optional button below the text which can be used to easily show the user
 * what to do next to add content to the view.
 *
 * The top-level component is a ColumnLayout, so additional components items can
 * simply be added as child items and they will be positioned sanely.
 *
 * Example usage:
 *
 * @code{.qml}
 ** Shows how to use PlaceholderMessage to implement a "this view is empty" message
 * import QtQuick 2.12
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 * import org.kde.plasma.core 2.0 as PlasmaCore
 *
 * ListView {
 *     id: listView
 *     model: [...]
 *     delegate: [...]
 *
 *     PlasmaExtras.PlaceholderMessage {
 *         anchors.centerIn: parent
 *         width: parent.width - (PlasmaCore.Units.largeSpacing * 4)
 *
 *         visible: listView.count == 0
 *
 *         text: "There are no items in this list"
 *     }
 * }
 * @endcode
 * @code{.qml}
 ** Shows how to use PlaceholderMessage to implement a "here's how to proceed" message
 * import QtQuick 2.12
 * import QtQuick.Controls 2.12 as QQC2
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 * import org.kde.plasma.core 2.0 as PlasmaCore
 *
 * ListView {
 *     id: listView
 *     model: [...]
 *     delegate: [...]
 *
 *     PlasmaExtras.PlaceholderMessage {
 *         anchors.centerIn: parent
 *         width: parent.width - (PlasmaCore.Units.largeSpacing * 4)
 *
 *         visible: listView.count == 0
 *
 *         text: "Add an item to proceed"
 *
 *         helpfulAction: QQC2.Action {
 *             icon.name: "list-add"
 *             text: "Add item..."
 *             onTriggered: {
 *                 [...]
 *             }
 *         }
 *     }
 *     [...]
 * }
 * @endcode
 * @code{.qml}
 ** Shows how to use PlaceholderMessage to implement a "there was a problem here" message
 * import org.kde.plasma.components 3.0 as PlasmaComponents3
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 * import org.kde.plasma.core 2.0 as PlasmaCore
 *
 * PlasmaComponents3.Page {
 *     id: root
 *     readonly property bool networkConnected: [...]
 *
 *     PlasmaExtras.PlaceholderMessage {
 *         anchors.centerIn: parent
 *         width: parent.width - (PlasmaCore.Units.largeSpacing * 4)
 *
 *         visible: root.networkConnected
 *
 *         iconName: "network-disconnect"
 *         text: "Unable to load content
 *         explanation: "Please try again later"
 *     }
 * }
 * @endcode
 * @code{.qml}
 * import org.kde.plasma.components 3.0 as PlasmaComponents3
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 * import org.kde.plasma.core 2.0 as PlasmaCore
 *
 ** Shows how to use PlaceholderMessage to implement a loading indicator
 * PlasmaComponents3.Page {
 *     id: root
 *     readonly property bool loading: [...]
 *     readonly property int completionStatus: [...]
 *
 *     PlasmaExtras.PlaceholderMessage {
 *         anchors.centerIn: parent
 *         width: parent.width - (PlasmaCore.Units.largeSpacing * 4)
 *
 *         visible: root.loading
 *
 *         iconName: "my-awesome-app-icon"
 *         text: "Loading this awesome app"
 *
 *         PlasmaComponents3.ProgressBar {
 *             Layout.preferredWidth: PlasmaCore.Units.gridUnit * 20
 *             value: root.completionStatus
 *             from: 0
 *             to: 100
 *         }
 *     }
 * }
 * @endcode
 * @code{.qml}
 * import QtQuick.Controls 2.12 as QQC2
 * import org.kde.plasma.components 3.0 as PlasmaComponents3
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 * import org.kde.plasma.core 2.0 as PlasmaCore
 *
 ** Shows how to use PlaceholderMessage to implement a "Here's what you do next" button
 * PlasmaComponents3.Page {
 *     id: root
 *
 *     PlasmaExtras.PlaceholderMessage {
 *         anchors.centerIn: parent
 *         width: parent.width - (PlasmaCore.Units.largeSpacing * 4)
 *
 *         visible: root.loading
 *
 *         helpfulAction: QQC2.Action {
 *             icon.name: "list-add"
 *             text: "Add item..."
 *             onTriggered: {
 *                 [...]
 *             }
 *         }
 *     }
 * }
 * @endcode
 * @since 5.72
 */
ColumnLayout {
    id: root

    /**
     * text: string
     * The text to show as a placeholder label
     *
     * Optional; if not defined, the message will have no large text label
     * text. If both text: and explanation: are omitted, the message will have
     * no text and only an icon, action button, and/or other custom content.
     *
     * @since 5.72
     */
    property string text

    /**
     * explanation: string
     * Smaller explanatory text to show below the larger title-style text
     *
     * Useful for providing a user-friendly explanation for how to proceed.
     *
     * Optional; if not defined, the message will have no supplementary
     * explanatory text.
     *
     * @since 5.80
     */
    property string explanation

    /**
     * iconName: string
     * The icon to show above the text label.
     *
     * Optional
     * Falls back to `undefined` if the specified icon is not valid or cannot
     * be loaded.
     *
     * @since 5.72
     * @see Icon::source
     */
    property string iconName

    /**
     * helpfulAction: QtQuickControls2 Action
     * An action that helps the user proceed. Typically used to guide the user
     * to the next step for adding content or items to an empty view.
     *
     * Optional
     *
     * @since 5.72
     */
    property alias helpfulAction: actionButton.action

    spacing: PlasmaCore.Units.largeSpacing

    PlasmaCore.IconItem {
        visible: source != undefined
        opacity: 0.5

        Layout.alignment: Qt.AlignHCenter
        Layout.preferredWidth: PlasmaCore.Units.iconSizes.huge
        Layout.preferredHeight: PlasmaCore.Units.iconSizes.huge

        source: root.iconName || null
    }

    PlasmaExtras.Heading {
        text: root.text
        visible: text.length > 0

        level: 2
        opacity: 0.5

        Layout.fillWidth: true
        horizontalAlignment: Qt.AlignHCenter

        wrapMode: Text.WordWrap
    }

    PlasmaComponents3.Label {
        text: root.explanation
        visible:  root.explanation !== ""

        opacity: 0.5

        horizontalAlignment: Qt.AlignHCenter
        wrapMode: Text.WordWrap

        Layout.fillWidth: true
    }

    PlasmaComponents3.Button {
        id: actionButton

        Layout.alignment: Qt.AlignHCenter

        visible: action && action.enabled
    }
}
