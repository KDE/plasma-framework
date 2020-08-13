/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore

import "." 2.0 as PlasmaComponents

/**
 * A plasma themed SelectionDialog, which can be customized by overriding the
 * ListView delegate.
 *
 * By default SelectionDialog provides a scrollable list of textual menu items.
 * The user can choose one item from the list at a time.
 */
PlasmaComponents.CommonDialog {
    id: root

    // Common API
    /**
     * type:QtObject
     * The model of selectionDialog. Can be a simple model or a custom
     * QAbstractItemModel
     */
    property alias model: listView.model

    /**
     * Selected index.
     *
     * The default value is -1.
     */
    property int selectedIndex: -1

    /**
     * The delegate used to render the items.
     */
    property Component delegate: defaultDelegate

    Component {
        id: defaultDelegate

        Label {
            visible: modelData.search(RegExp(filterField.filterText, "i")) != -1
            height: visible? paintedHeight*2 : 0
            text: modelData
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    selectedIndex = index
                    root.accept()
                }
            }

            Keys.onPressed: {
                if (event.key == Qt.Key_Up || event.key == Qt.Key_Down)
                    scrollBar.flash()
            }
        }
    }

    content: Item {
        id: contentItem
        property alias filterText: filterField.filterText
        implicitWidth: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 40
        implicitHeight: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * 12
        height: implicitHeight

        TextField {
            id: filterField
            property string filterText
            onTextChanged: searchTimer.restart()
            clearButtonShown: true
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
            Timer {
                id: searchTimer
                running: false
                repeat: false
                interval: 500
                onTriggered: filterField.filterText = filterField.text
            }
        }
        ListView {
            id: listView

            anchors {
                top: filterField.bottom
                left: parent.left
                right: parent.right
                bottom: parent.bottom
            }
            currentIndex : -1
            delegate: root.delegate
            clip: true

            Keys.onPressed: {
                if (event.key == Qt.Key_Up || event.key == Qt.Key_Down
                    || event.key == Qt.Key_Left || event.key == Qt.Key_Right
                    || event.key == Qt.Key_Select || event.key == Qt.Key_Enter
                    || event.key == Qt.Key_Return) {
                    listView.currentIndex = 0
                    event.accepted = true
                }
            }
        }

        PlasmaComponents.ScrollBar {
            id: scrollBar
            flickableItem: listView
            visible: listView.contentHeight > contentItem.height
            //platformInverted: root.platformInverted
            anchors { top: contentItem.top; right: contentItem.right }
        }
    }

    onClickedOutside: {
        reject()
    }

    Timer {
        id: focusTimer
        interval: 100
        onTriggered: {
            filterField.forceActiveFocus()
        }
    }
    onStatusChanged: {
        //FIXME: why needs focus deactivation then activation?
        if (status == PlasmaComponents.DialogStatus.Open) {
            filterField.focus = false
            focusTimer.running = true
        }

        if (status == PlasmaComponents.DialogStatus.Opening) {
            if (listView.currentItem != null) {
                listView.currentItem.focus = false
            }
            listView.currentIndex = -1
            listView.positionViewAtIndex(0, ListView.Beginning)
        } else if (status == PlasmaComponents.DialogStatus.Open) {
            listView.focus = true
        }
    }
}
