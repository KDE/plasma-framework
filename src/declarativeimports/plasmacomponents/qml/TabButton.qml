/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import "private/AppManager.js" as Utils
import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * A simple tab button which is using the plasma theme.
 *
 * @inherit QtQuick.Item
 */
Item {
    id: root

    // Common Public API
    /**
     * The reference to the tab content (one of the children of a TabGroup,
     * usually a Page) that is activated when this TabButton is clicked.
     */
    property Item tab

    /**
     * True if the button is checked, false otherwise.
     */
    property bool checked: (internal.tabGroup == null) ? (internal.tabBar && internal.tabBar.currentTab == root) : (internal.tabGroup.currentTab == tab)

    /**
     * True if the button is being pressed, false otherwise.
     */
    property bool pressed: mouseArea.pressed == true && mouseArea.containsMouse

    /**
     * type:string
     * The text for the button.
     */
    property alias text: label.text

    /**
     * type:string
     * Icon for the button. It can be a Freedesktop icon name, a full path to a
     * png/svg file, or any name for which the application has an image handler
     * registered.
     */
    property alias iconSource: imageLoader.source

    /**
     * Emitted when the button is clicked.
     */
    signal clicked

    Accessible.role: Accessible.PageTab
    Accessible.checkable: true
    Accessible.checked: checked
    Accessible.name: text

    implicitWidth: Math.max(label.implicitWidth + (internal.portrait ? 0 : (iconSource != null ? PlasmaCore.Units.iconSizes.small : 0)), height)
    implicitHeight: 20//label.implicitHeight + (internal.portrait ? (iconSource != null ? PlasmaCore.Units.iconSizes.small : 0) : 0)

    opacity: enabled ? 1 : 0.6
    //long notation to not make it overwritten by implementations
    Connections {
        target: (root != undefined) ? root : undefined
        function onPressedChanged() {
            //TabBar is the granparent
            internal.tabBar.currentTab = root
            internal.tabBar.forceActiveFocus()
        }
        function onClicked() {
            if (internal.tabGroup) {
                internal.tabGroup.currentTab = tab
            }
            //TabBar is the granparent, done here too in case of no tabgroup
            internal.tabBar.currentTab = root
        }
        function onVisibleChanged() {
            root.parent.childrenChanged()
        }
        function onTextChanged() {
            root.parent.childrenChanged()
        }
    }

    QtObject {
        id: internal

        property Item tabBar: Utils.findParent(root, "currentTab")
        property Item tabGroup: Utils.findParent(tab, "currentTab")
        property bool portrait: (root != undefined) && (label != undefined) &&  label.text != "" && root.height >= label.paintedHeight + PlasmaCore.Units.iconSizes.small

        function click() {
            root.clicked()
            if (internal.tabGroup) {
                internal.tabGroup.currentTab = tab
            }
        }

        Component.onCompleted: {
            if (internal.tabGroup && internal.tabGroup.currentTab == tab) {
                internal.tabGroup.currentTab = tab
            }
        }
    }

    GridLayout {
        anchors.fill: parent
        rows: 1
        columns: 1
        rowSpacing: 0
        columnSpacing: 0
        flow: internal.portrait ? GridLayout.LeftToRight : GridLayout.TopToBottom

        PlasmaCore.IconItem {
            id: imageLoader
            visible: iconSource != null
            colorGroup: PlasmaCore.ColorScope.colorGroup
            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter

            implicitWidth: internal.portrait ? Math.max(PlasmaCore.Units.iconSizes.small, root.height - (label.text ? label.height : 0)) : Math.max(PlasmaCore.Units.iconSizes.small, root.height)
            implicitHeight: implicitWidth

        }

        Label {
            id: label

            objectName: "label"
            Accessible.ignored: true

            Layout.alignment: Qt.AlignVCenter | Qt.AlignHCenter
            Layout.preferredWidth: internal.portrait ? root.width : implicitWidth
            Layout.preferredHeight: internal.portrait ? implicitHeight : root.height

            //elide: Text.ElideRight
            horizontalAlignment: !internal.portrait && iconSource != null ? Text.AlignLeft : Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter

            color: PlasmaCore.ColorScope.textColor
        }
    }

    MouseArea {
        id: mouseArea

        onClicked: root.clicked()

        anchors.fill: parent
    }
}
