/****************************************************************************
**
** Copyright (C) 2011 Marco Martin  <mart@kde.org>
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Components project.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import "private/AppManager.js" as Utils
import "." 2.0 as PlasmaComponents

/**
 * Provides a top-level window for short-term tasks and brief interaction with
 * the user.
 *
 * It is intended to be used for interaction more complex and bigger in size
 * than Dialog. On the desktop its looks is almost identical to Dialog, on
 * touch interfaces it is an almost fullscreen sliding Sheet. It is provided
 * mostly for compatibility with mobile implementations.
 *
 * @inherit QtQuick.Item
 */
Item {
    id: root

    /**
     * type:string
     * The title text of this Sheet.
     */
    property alias title: titleLabel.text

    /**
     * type:list<Item>
     * A list of items in the dialog's content area. You can use any component
     * that is based on Item. For example, you can use ListView, so that the
     * user can select from a list of names.
     */
    property alias content: contentItem.children
//    property alias visualParent: dialog.visualParent

    /**
     * Indicates the dialog's phase in its life cycle. The values are as follows:
     *
     * - DialogStatus.Opening - the dialog is opening
     * - DialogStatus.Open - the dialog is open and visible to the user
     * - DialogStatus.Closing - the dialog is closing
     * - DialogStatus.Closed - the dialog is closed and not visible to the user
     *
     * The dialog's initial status is DialogStatus.Closed.
     */
    property int status: PlasmaComponents.DialogStatus.Closed
    property alias acceptButtonText: acceptButton.text
    property alias rejectButtonText: rejectButton.text
    /**
     * type:Button
     * button that when pressed will close the dialog, representing the user
     * accepting it, accepted() will be called
     */
    property alias acceptButton: acceptButton

    /**
     * type:Button
     * button that when pressed will close the dialog, representing the user rejecting it, rejected() will be called
     */
    property alias rejectButton: rejectButton

    property alias privateTitleHeight: titleBar.height
    property alias privateButtonsHeight: buttonsRow.height

    /**
     * Emitted when the user accepts the dialog's request or the accept()
     * method is called.
     */
    signal accepted

    /**
     * Emitted when the user rejects the dialog's request or the reject()
     * method is called.
     *
     * @see accepted()
     */
    signal rejected

    /**
     * Emitted when the user taps in the area that is inside the dialog's
     * visual parent area but outside the dialog's area. Normally the visual
     * parent is the root object. In that case this signal is emitted if the
     * user taps anywhere outside the dialog's area.
     */
    signal clickedOutside

    /**
     * Shows the dialog to the user.
     */
    function open()
    {
        dialog.visible = true
        dialog.requestActivate()
    }

    /**
     * Accepts the dialog's request without any user interaction. The method
     * emits the accepted() signal and closes the dialog.
     *
     * @see reject()
     */
    function accept()
    {
        if (status == DialogStatus.Open) {
            dialog.visible = false
            accepted()
        }
    }

    /**
     * Rejects the dialog's request without any user interaction. The method
     * emits the rejected() signal and closes the dialog.
     *
     * @see accept()
     */
    function reject() {
        if (status == DialogStatus.Open) {
            dialog.visible = false
            rejected()
        }
    }

    /**
     * Closes the dialog without any user interaction.
     */
    function close() {
        dialog.visible = false
    }

    visible: false

    PlasmaCore.Dialog {
        id: dialog
        flags: Qt.Dialog
        location: PlasmaCore.Types.BottomEdge


        //onFaderClicked: root.clickedOutside()
        property Item rootItem

        //state: "Hidden"
        visible: false
        onVisibleChanged: {
            if (visible) {
                status = PlasmaComponents.DialogStatus.Open
            } else {
                status = PlasmaComponents.DialogStatus.Closed
            }
        }

        mainItem: Item {
            id: mainItem
            width: theme.mSize(theme.defaultFont).width * 40
            height: Math.max(titleBar.childrenRect.height + contentItem.childrenRect.height + buttonsRow.childrenRect.height + 8, theme.mSize(theme.defaultFont).height * 25)

            // Consume all key events that are not processed by children
            Keys.onPressed: event.accepted = true
            Keys.onReleased: event.accepted = true

            PlasmaCore.FrameSvgItem {
                id: titleBar
                imagePath: "widgets/extender-dragger"
                prefix: "root"
                anchors.left: parent.left
                anchors.right: parent.right
                //FIXME: +5 because of Plasma::Dialog margins
                height: titleLabel.paintedHeight + margins.top + margins.bottom

                Column {
                    id: titleLayoutHelper // needed to make the text mirror correctly

                    anchors {
                        right: parent.right
                        left: parent.left
                        top: parent.top
                        bottom: parent.bottom
                        leftMargin: parent.margins.left
                        rightMargin: parent.margins.right
                        topMargin: parent.margins.top
                        bottomMargin: parent.margins.bottom
                    }

                    PlasmaComponents.Label {
                        id: titleLabel
                        elide: Text.ElideRight
                        height: paintedHeight
                        font.pointSize: theme.defaultFont.pointSize * 1.1
                        font.weight: Font.Bold
                        style: Text.Raised
                        styleColor: Qt.rgba(1,1,1,0.8)
                        anchors {
                            left: parent.left
                            right: parent.right
                        }
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                }
            }

            Item {
                id: contentItem

                onChildrenRectChanged: mainItem.width = Math.max(childrenRect.width, buttonsRow.childrenRect.width)

                clip: true
                anchors {
                    top: titleBar.bottom
                    left: parent.left
                    right: parent.right
                    bottom: buttonsRow.top
                    bottomMargin: 8
                }
            }

            Row {
                id: buttonsRow
                spacing: 8
                anchors {
                    bottom: parent.bottom
                    horizontalCenter: parent.horizontalCenter
                    //the bottom margin is disabled but we want it anyways
                    bottomMargin: theme.mSize(theme.defaultFont).height*0.6
                }

                PlasmaComponents.Button {
                    id: acceptButton
                    onClicked: accept()
                }

                PlasmaComponents.Button {
                    id: rejectButton
                    onClicked: reject()
                }
            }
        }

        Component.onCompleted: {
            rootItem = Utils.rootObject(this)
        }
    }
}
