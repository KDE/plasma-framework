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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import "private/AppManager.js" as Utils
import "." 0.1

/**Documented API
Inherits:
        Item

Imports:
        QtQuick 1.0
        org.kde.plasma.core

Description:
        Provides a top-level window for short-term tasks and brief interaction with the user.
        Is intended to be for interaction more complex and bigger in size then Dialog. On the desktop its looks is almost identical to Dialog, on touch interfaces is an almost fullscreen sliding Sheet. It is provided mostly for compatibility with mobile implementations

Properties:
        list<Item> content:
        A list of items in the dialog's content area. You can use any component that is based on Item. For example, you can use ListView, so that the user can select from a list of names.

        int status:
        Indicates the dialog's phase in its life cycle. The values are as follows:
            - DialogStatus.Opening - the dialog is opening
            - DialogStatus.Open - the dialog is open and visible to the user
            - DialogStatus.Closing - the dialog is closing
            - DialogStatus.Closed - the dialog is closed and not visible to the user
        The dialog's initial status is DialogStatus.Closed.

        string title:
        The title text of this Sheet.

        Item acceptButton:
        button that when pressed will close the dialog, representing the user accepting it, accepted() will be called

        Item rejectButton:
        button that when pressed will close the dialog, representing the user rejecting it, rejected() will be called

        string acceptButtonText:
        Text of the accept button

        string rejectButtonText:
        Text of the reject button

        Item visualParent:
        The item that is dimmed when the dialog opens. By default the root parent object is visualParent.

Signals:
        accepted():
        This signal is emitted when the user accepts the dialog's request or the accept() method is called.
        See also rejected().

        clickedOutside(): This signal is emitted when the user taps in the area that is inside the dialog's visual parent area but outside the dialog's area. Normally the visual parent is the root object. In that case this signal is emitted if the user taps anywhere outside the dialog's area.
        See also visualParent.

        rejected():
        This signal is emitted when the user rejects the dialog's request or the reject() method is called.
        See also accepted().

Methods:
        void accept():
        Accepts the dialog's request without any user interaction. The method emits the accepted() signal and closes the dialog.
        See also reject().

        void close():
        Closes the dialog without any user interaction.

        void open():
        Shows the dialog to the user.

        void reject():
        Rejects the dialog's request without any user interaction. The method emits the rejected() signal and closes the dialog.
        See also accept().
**/

Item {
    id: root
    width: 800
    height: 600
    onHeightChanged:print(height)

    property alias title: titleLabel.text
    property alias content: contentItem.children
//    property alias visualParent: dialog.visualParent
    property int status: DialogStatus.Closed
    property alias acceptButtonText: acceptButton.text
    property alias rejectButtonText: rejectButton.text
    property alias acceptButton: acceptButton
    property alias rejectButton: rejectButton


    property alias privateTitleHeight: titleBar.height

    signal accepted
    signal rejected
    signal clickedOutside

    function open()
    {
        status = DialogStatus.Opening
        delayOpenTimer.restart()
    }

    function accept()
    {
        if (status == DialogStatus.Open) {
            status = DialogStatus.Closing
            accepted()
            dialog.state = "closed"
        }
    }

    function reject()
    {
        if (status == DialogStatus.Open) {
            status = DialogStatus.Closing
            dialog.state = "closed"
            rejected()
        }
    }

    function close()
    {
        dialog.state = "closed"
    }


    MouseArea {
        anchors.fill: parent
        onClicked: {
            clickedOutside()
            close()
        }
    }
    Timer {
        id: delayOpenTimer
        running: false
        interval: 100
        onTriggered: dialog.state = ""
    }

    PlasmaCore.FrameSvgItem {
        id: dialog
        anchors {
            fill: parent
            leftMargin: 50
            topMargin: 50
            rightMargin: 50
            bottomMargin: 0
        }
        imagePath: "dialogs/background"
        enabledBorders: "LeftBorder|TopBorder|RightBorder"

        state: "closed"

        //state: "Hidden"

        MouseArea {
            id: mainItem
            x: dialog.margins.left
            y: dialog.margins.top
            width: parent.width - dialog.margins.left - dialog.margins.right
            height: parent.height - dialog.margins.top - dialog.margins.bottom

            onClicked: mouse.accepted = true

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
                height: Math.max(titleLabel.paintedHeight, acceptButton.height) + margins.top + margins.bottom

                Item {
                    id: titleLayoutHelper

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

                    Button {
                        id: acceptButton
                        onClicked: accept()
                        visible: text !== ""
                        anchors {
                            left: parent.left
                            verticalCenter: parent.verticalCenter
                        }
                    }
                    Label {
                        id: titleLabel
                        elide: Text.ElideRight
                        height: paintedHeight
                        font.pointSize: theme.defaultFont.pointSize * 1.1
                        font.weight: Font.Bold
                        style: Text.Raised
                        styleColor: Qt.rgba(1,1,1,0.8)
                        anchors {
                            left: acceptButton.visible ? acceptButton.right : parent.left
                            //still depends from acceptButton to make text more centered
                            right: acceptButton.visible ? rejectButton.left : parent.right
                            verticalCenter: parent.verticalCenter
                        }
                        horizontalAlignment: Text.AlignHCenter
                        verticalAlignment: Text.AlignVCenter
                    }
                    Button {
                        id: rejectButton
                        onClicked: reject()
                        visible: text !== ""
                        anchors {
                            right: parent.right
                            verticalCenter: parent.verticalCenter
                        }
                    }
                }
            }

            Item {
                id: contentItem

                clip: true
                onChildrenRectChanged: mainItem.width = childrenRect.width
                anchors {
                    top: titleBar.bottom
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
            }

        }

        states: [
            State {
                name: "closed"
                PropertyChanges {
                    target: root
                    status: DialogStatus.Closed
                }
            },
            State {
                name: ""
                PropertyChanges {
                    target: root
                    status: DialogStatus.Open
                }
            }
        ]

    }
}
