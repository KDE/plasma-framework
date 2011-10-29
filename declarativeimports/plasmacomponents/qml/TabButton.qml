/****************************************************************************
**
** Copyright 2011 Marco Martin <mart@kde.org>
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

import QtQuick 1.1
import "AppManager.js" as Utils

Item {
    id: root

    // Common Public API
    property Item tab
    property bool checked: internal.tabGroup != null && internal.tabGroup.currentTab == tab
    property bool pressed: mouseArea.pressed == true && mouseArea.containsMouse
    property alias text: label.text
    property alias iconSource: imageLoader.source

    signal clicked

    implicitWidth: label.paintedWidth + (internal.portrait ? 0 : (iconSource != "" ? 16 : 0))
    implicitHeight: label.paintedHeight + (internal.portrait ? (iconSource != "" ? 16 : 0) : 0)

    //long notation to not make it overwritten by implementations
    Connections {
        target: root
        onPressedChanged: {
            //TabBar is the granparent
            root.parent.parent.currentItem = root
        }
    }

    QtObject {
        id: internal

        property Item tabGroup: Utils.findParent(tab, "currentTab")
        property bool portrait: root.height >= label.paintedHeight + 16

        function click() {
            root.clicked()
            if (internal.tabGroup) {
                internal.tabGroup.currentTab = tab
            }
        }

        Component.onCompleted: {
            if (internal.tabGroup.currentTab == tab) {
                parent.parent.currentItem = root
            }
        }
    }

    Label {
        id: label

        objectName: "label"

        anchors {
            top: internal.portrait && iconSource != "" ? imageLoader.bottom : parent.top
            left: internal.portrait || iconSource == "" ? parent.left : imageLoader.right
            right: parent.right
            bottom: parent.bottom
        }

        elide: Text.ElideRight
        horizontalAlignment: !internal.portrait && iconSource != "" ? Text.AlignLeft : Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
    }

    IconLoader {
        id: imageLoader

        //FIXME: icon sizes in Theme
        width : 16
        height : 16

        anchors {
            left: internal.portrait ? undefined : parent.left
            horizontalCenter: internal.portrait ? parent.horizontalCenter : undefined
            verticalCenter: internal.portrait ? undefined : parent.verticalCenter
        }
    }

    MouseArea {
        id: mouseArea

        onClicked: {
            root.clicked()
            if (internal.tabGroup) {
                internal.tabGroup.currentTab = tab
            }
            //TabBar is the granparent, done here too in case of no tabgroup
            root.parent.parent.currentItem = root
        }

        anchors.fill: parent
    }
}
