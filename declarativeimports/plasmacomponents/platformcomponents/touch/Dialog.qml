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
import "AppManager.js" as Utils
import "." 0.1

Item {
    id: root

    property alias title: titleBar.children
    property alias content: contentItem.children
    property alias buttons: buttonItem.children
//    property alias visualParent: dialog.visualParent
    property int status: DialogStatus.Closed


    property alias privateTitleHeight: titleBar.height
    property alias privateButtonsHeight: buttonItem.height

    signal accepted
    signal rejected
    signal clickedOutside

    function open()
    {
        var pos = dialog.popupPosition(null, Qt.alignCenter)
        dialog.x = pos.x
        dialog.y = pos.y

        dialog.visible = true
        dialog.activateWindow()
    }

    function accept()
    {
        if (status == DialogStatus.Open) {
            dialog.visible = false
            accepted()
        }
    }

    function reject() {
        if (status == DialogStatus.Open) {
            dialog.visible = false
            rejected()
        }
    }

    function close() {
        dialog.visible = false
    }

    visible: false

    PlasmaCore.Dialog {
        id: dialog
        windowFlags: Qt.Dialog

        //onFaderClicked: root.clickedOutside()
        property Item rootItem

        //state: "Hidden"
        visible: false
        onVisibleChanged: {
            if (visible) {
                status = DialogStatus.Open
            } else {
                status = DialogStatus.Closed
            }
        }
        onActiveWindowChanged: if (!activeWindow) dialog.visible = false

        mainItem: Item {
            id: mainItem
            width: theme.defaultFont.mSize.width * 40
            height: titleBar.childrenRect.height + contentItem.childrenRect.height + buttonItem.childrenRect.height

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

                onChildrenRectChanged: mainItem.width = childrenRect.width
                clip: true
                anchors {
                    top: titleBar.bottom
                    left: parent.left
                    right: parent.right
                    bottom: buttonItem.top
                }
            }

            Item {
                id: buttonItem

                height: childrenRect.height
                clip: true
                anchors {
                    left: parent.left
                    right: parent.right
                    bottom: parent.bottom
                }
            }
        }

        Component.onCompleted: {
            rootItem = Utils.rootObject()
        }
    }
}
