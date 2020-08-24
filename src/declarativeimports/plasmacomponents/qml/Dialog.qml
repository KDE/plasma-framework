/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import "private/AppManager.js" as Utils
import org.kde.plasma.components 2.0 as PlasmaComponents
import "private" as Private

/**
 * Top-level window for short-term tasks and brief interaction with the user
 *
 * A dialog floats on the top layer of the application view, usually
 * overlapping the area reserved for the application content. Normally, a
 * dialog provides information and gives warnings to the user, or asks the user
 * to answer a question or select an option.
 *
 * @inherit QtQuick.Item
 */
Item {
    id: root

    /**
     * type:list<Item> A list of items in the dialog's title area. You can use
     * a Text component but also any number of components that are based on
     * Item. For example, you can use Text and Image components.
     */
    property alias title: titleBar.children

    /**
     * type:list<Item> A list of items in the dialog's content area. You can
     * use any component that is based on Item. For example, you can use
     * ListView, so that the user can select from a list of names.
     */
    property alias content: contentItem.children

    /**
     * type:list<Item> A list of items in the dialog's button area. For
     * example, you can use Row or Button components but you can also use any
     * number of components that are based on Item component.
     */
    property alias buttons: buttonItem.children

    /**
     * The item that the dialog refers to. The dialog will usually be
     * positioned relative to VisualParent
     */
    property Item visualParent

    /**
     * Indicates the dialog's phase in its life cycle. The values are as follows:
     *
     * - PlasmaComponents.DialogStatus.Opening - the dialog is opening
     * - PlasmaComponents.DialogStatus.Open - the dialog is open and visible to the user
     * - PlasmaComponents.DialogStatus.Closing - the dialog is closing
     * - PlasmaComponents.DialogStatus.Closed - the dialog is closed and not visible to the user
     *
     * The dialog's initial status is PlasmaComponents.DialogStatus.Closed.
     */
    property int status: PlasmaComponents.DialogStatus.Closed


    property alias privateTitleHeight: titleBar.height
    property alias privateButtonsHeight: buttonItem.height

    /**
     * Hint on where to display the dialog in relation to visualParent
     */
    property int location: PlasmaCore.Types.Floating

    /**
     * Hide dialog when focus is lost.
     */
    property bool hideOnWindowDeactivate: false

    /**
     * This signal is emitted when the user accepts the dialog's request or the
     * accept() method is called.
     *
     * @see rejected()
     */
    signal accepted

    /**
     *
     * This signal is emitted when the user rejects the dialog's request or the
     * reject() method is called.
     *
     * @see accepted()
     */

    signal rejected

    /**
     * This signal is emitted when the user taps in the area that is inside the
     * dialog's visual parent area but outside the dialog's area. Normally the
     * visual parent is the root object. In that case this signal is emitted if
     * the user taps anywhere outside the dialog's area.
     *
     * @see visualParent
     */
    signal clickedOutside

    function showObj(o) {
        print("Showing object: " + o);
        var e;
        for (e in o) {
            print(" e[" + e + "] = " + o[e]);
        }
    }


    /**
     * Shows the dialog to the user.
     */
    function open() {
        dialogLayout.parent = internalLoader.item.mainItem

        print("opening.");
        if (internalLoader.dialog) {
            print("Have dialog.");
//             var pos = internalLoader.dialog.popupPosition(root.visualParent, Qt.AlignCenter)
//             internalLoader.dialog.x = pos.x
//             internalLoader.dialog.y = pos.y

            internalLoader.dialog.visible = true
            internalLoader.dialog.requestActivate();
            //internalLoader.dialog.open();
        } else {
            print("inlining.");
            internalLoader.inlineDialog.open()
        }
    }

    /**
     * Accepts the dialog's request without any user interaction. The method
     * emits the accepted() signal and closes the internalLoader.dialog.
     *
     * @see reject()
     */
    function accept() {
        if (status == PlasmaComponents.DialogStatus.Open) {
//             coreDialog.visible = false;
            if (internalLoader.dialog) {
                internalLoader.dialog.visible = false
            } else {
                internalLoader.inlineDialog.close()
            }
            accepted()
        }
    }

    /**
     * Rejects the dialog's request without any user interaction. The method
     * emits the rejected() signal and closes the internalLoader.dialog.
     *
     * @see accept()
     */
    function reject() {
        if (status == PlasmaComponents.DialogStatus.Open) {
//             coreDialog.visible = false;
            if (internalLoader.dialog) {
                internalLoader.dialog.visible = false
            } else {
                internalLoader.inlineDialog.close()
            }
            rejected()
        }
    }

    /**
     * Closes the dialog without any user interaction.
     */
    function close() {
        //internalLoader.sourceComponent.
        //coreDialog.visible = false;
        if (internalLoader.dialog) {
            internalLoader.dialog.visible = false
        } else {
            internalLoader.inlineDialog.close()
        }
    }

    visible: false

    Loader {
        id: internalLoader
        //the root item of the scene. Determines if there is enough room for an inline dialog
        property Item rootItem

        //this is when the dialog is a separate window
        property QtObject dialog: sourceComponent == dialogComponent ? item : null
        //this is when the dialog is inline
        property Item inlineDialog: sourceComponent == inlineDialogComponent ? item : null

        property bool loadCompleted: false

        Component.onCompleted: {
            rootItem = Utils.rootObject(this)
            loadCompleted = true
        }

        sourceComponent: {
            //print(" XXX Loading Source component XXX");
            //print(" root: " + rootItem.width + "x" + rootItem.height + " dlg: " + dialogLayout.width + "x" + dialogLayout.height);
            if (loadCompleted) {
                if (true || rootItem == null || dialogLayout.width > rootItem.width || dialogLayout.height > rootItem.height) {
                    print(" in external window");
                    dialogComponent
                } else {
                    print(" inline window");
                    inlineDialogComponent
                }
            }
        }
    }

    Component {
        id: dialogComponent
        //property bool visible: coreDialog.visible
        PlasmaCore.Dialog {
            id: coreDialog
            //windowFlags: Qt.Popup
            visualParent: root.visualParent
            hideOnWindowDeactivate: root.hideOnWindowDeactivate
            location: root.location
            visible: false
            onVisibleChanged: {
                if (visible) {
                    root.status = PlasmaComponents.DialogStatus.Open
                } else {
                    root.status = PlasmaComponents.DialogStatus.Closed
                }
            }

            mainItem: Item {
                id: dialogMainItem
                width: dialogLayout.width
                height: dialogLayout.height
            }

            Component.onCompleted: dialogLayout.parent = dialogMainItem
            Component.onDestruction: dialogLayout.parent = root
        }
    }

    Component {
        id: inlineDialogComponent
        Private.InlineDialog {
            id: inlineDialog
            visualParent: root.visualParent
            property Item mainItem: inlineDialogMainItem
            onStatusChanged: root.status = status

            Item {
                id: inlineDialogMainItem
                width: dialogLayout.width
                height: dialogLayout.height
            }

            Component.onCompleted: {
                dialogLayout.parent = inlineDialogMainItem
            }
            Component.onDestruction: dialogLayout.parent = root
        }
    }

    Item {
        id: dialogLayout
        width: Math.max(buttonItem.childrenRect.width,  contentItem.childrenRect.width)
        height: titleBar.height + contentItem.childrenRect.height + buttonItem.childrenRect.height + 10

        parent: internalLoader.dialog ? internalLoader.dialog : internalLoader.inlineDialog
        // Consume all key events that are not processed by children
        Keys.onPressed: event.accepted = true
        Keys.onReleased: event.accepted = true

        Item {
            id: titleBar

            height: children.length > 0 && children[0].visible ? childrenRect.height : 0
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
            }
        }

        Item {
            id: contentItem

            clip: true
            anchors {
                top: titleBar.bottom
                left: parent.left
                right: parent.right
                bottom: buttonItem.top
                bottomMargin: 6
            }
        }

        Item {
            id: buttonItem

            height: childrenRect.height
            anchors {
                left: parent.left
                right: parent.right
                bottom: parent.bottom
                bottomMargin: 4
            }
        }
    }
}
