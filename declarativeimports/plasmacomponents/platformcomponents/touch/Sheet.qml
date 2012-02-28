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

Item {
    id: root
    width: 600
    height: 200
    onHeightChanged:print(height)

    property alias title: titleBar.children
    property alias content: contentItem.children
//    property alias visualParent: dialog.visualParent
    property int status: DialogStatus.Closed
    property alias acceptButtonText: acceptButton.text
    property alias rejectButtonText: rejectButton.text
    property alias acceptButton: acceptButton
    property alias rejectButton: rejectButton


    property alias privateTitleHeight: titleBar.height
    property alias privateButtonsHeight: buttonsRow.height

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
        anchors.fill: parent
        imagePath: "dialogs/background"

        state: "closed"

        //state: "Hidden"

        Item {
            id: mainItem
            x: dialog.margins.left
            y: dialog.margins.top
            width: parent.width - dialog.margins.left - dialog.margins.right
            height: parent.height - dialog.margins.top - dialog.margins.bottom

            // Consume all key events that are not processed by children
            Keys.onPressed: event.accepted = true
            Keys.onReleased: event.accepted = true

            Item {
                id: titleBar

                height: childrenRect.height
                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                }
            }

            Item {
                id: contentItem

                clip: true
                onChildrenRectChanged: mainItem.width = Math.max(childrenRect.width, buttonsRow.childrenRect.width)
                anchors {
                    top: titleBar.bottom
                    left: parent.left
                    right: parent.right
                    bottom: buttonsRow.top
                }
            }

            Row {
                id: buttonsRow
                spacing: 8
                anchors {
                    bottom: parent.bottom
                    horizontalCenter: parent.horizontalCenter
                    //the bottom margin is disabled but we want it anyways
                    bottomMargin: theme.defaultFont.mSize.height*0.6
                }

                Button {
                    id: acceptButton
                    onClicked: accept()
                }

                Button {
                    id: rejectButton
                    onClicked: reject()
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
