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
import "private/AppManager.js" as Utils
import org.kde.plasma.core 0.1 as PlasmaCore

/**
 * A simple tab button which is using the plasma theme.
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
    property bool checked: (internal.tabGroup == null) ? (internal.tabBar.currentTab == root) : (internal.tabGroup.currentTab == tab)

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

    implicitWidth: label.implicitWidth + (internal.portrait ? 0 : (iconSource != null ? 16 : 0))
    implicitHeight: label.implicitHeight + (internal.portrait ? (iconSource != null ? 16 : 0) : 0)

    opacity: enabled ? 1 : 0.6
    //long notation to not make it overwritten by implementations
    Connections {
        target: root
        onPressedChanged: {
            //TabBar is the granparent
            internal.tabBar.currentTab = root
            internal.tabBar.forceActiveFocus()
        }
        onClicked: {
            if (internal.tabGroup) {
                internal.tabGroup.currentTab = tab
            }
            //TabBar is the granparent, done here too in case of no tabgroup
            internal.tabBar.currentTab = root
        }
        onVisibleChanged: root.parent.childrenChanged()
    }

    QtObject {
        id: internal

        property Item tabBar: Utils.findParent(root, "currentTab")
        property Item tabGroup: Utils.findParent(tab, "currentTab")
        property bool portrait: root.height >= label.paintedHeight + 16

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

    Label {
        id: label

        objectName: "label"

        anchors {
            top: internal.portrait && iconSource != null ? imageLoader.bottom : parent.top
            left: internal.portrait || iconSource == null ? parent.left : imageLoader.right
            leftMargin: iconSource == null ? 0 : theme.defaultFont.mSize.width
            right: parent.right
            bottom: parent.bottom
        }

        elide: Text.ElideRight
        horizontalAlignment: !internal.portrait && iconSource != null ? Text.AlignLeft : Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter

        color: root.checked ? theme.buttonTextColor : theme.textColor
    }

    PlasmaCore.IconItem {
        id: imageLoader

        implicitWidth: internal.portrait ? Math.max(theme.smallIconSize, root.height - (label.text ? label.height : 0)) : Math.max(theme.smallIconSize, root.height)
        implicitHeight: implicitWidth

        anchors {
            left: internal.portrait ? undefined : parent.left
            horizontalCenter: internal.portrait ? parent.horizontalCenter : undefined
            verticalCenter: internal.portrait ? undefined : parent.verticalCenter
        }
    }

    MouseArea {
        id: mouseArea

        onClicked: root.clicked()

        anchors.fill: parent
    }
}
