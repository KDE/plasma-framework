/****************************************************************************
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
import "TabGroup.js" as Engine

import "." 0.1

Item {
    id: root
    property Item currentTab

    property list<Item> privateContents
    // Qt defect: cannot have list as default property
    default property alias privateContentsDefault: root.privateContents

    onChildrenChanged: {
        //  [0] is containerHost
        if (children.length > 1)
            Engine.addTab(children[1])
    }

    onPrivateContentsChanged: {
        Engine.ensureContainers()
    }

    Component.onCompleted: {
        // Set first tabs as current if currentTab is not set by application
        if (currentTab == null && containerHost.children[0] && containerHost.children[0].children[0])
            currentTab = containerHost.children[0].children[0]
        priv.complete = true;
    }

    Item {
        id: containerHost
        objectName: "containerHost"
        anchors.fill: parent
    }

    Component {
        id: tabContainerComponent
        Item {
            id: tabContainerItem

            onChildrenChanged: {
                if (children.length == 0)
                    Engine.removeContainer(tabContainerItem)

                else if (children.length == 1) {
                    children[0].width = width
                    children[0].height = height
                    // tab content created. set the first tab as current (if not set before, and if
                    // child is added after TabGroup has completed)
                    if (priv.complete && root.currentTab == null)
                        root.currentTab = children[0]
                }
            }

            onWidthChanged: {
                if (children.length > 0)
                    children[0].width = width
            }

            onHeightChanged: {
                if (children.length > 0)
                    children[0].height = height
            }

            Component.onDestruction: {
                if (typeof(root) != "undefined" && !root.currentTab) {
                    // selected one deleted. try to activate the neighbour
                    var removedIndex = -1
                    for (var i = 0; i < containerHost.children.length; i++) {
                        if (containerHost.children[i] == tabContainerItem) {
                            removedIndex = i
                            break
                        }
                    }
                    var newIndex = -1
                    if (removedIndex != -1) {
                        if (removedIndex != containerHost.children.length - 1)
                            newIndex = removedIndex + 1
                        else if (removedIndex != 0)
                            newIndex = removedIndex - 1
                    }

                    if (newIndex != -1)
                        root.currentTab = containerHost.children[newIndex].children[0]
                    else
                        root.currentTab = null
                }
            }

            function incomingDone() {
                state = ""
                if (priv.incomingPage) {
                    priv.incomingPage.status = PageStatus.Active
                    priv.incomingPage = null
                }
            }

            function outgoingDone() {
                if (priv.outgoingPage) {
                    priv.outgoingPage.status = PageStatus.Inactive
                    priv.outgoingPage.visible = false
                    priv.outgoingPage = null
                }
                state = "Hidden"
            }

            width: parent ? parent.width : 0
            height: parent ? parent.height : 0
            state: "Hidden"

            states: [
                State { name: ""; PropertyChanges { target: tabContainerItem; opacity: 1.0; x: 0 } },
                State { name: "Incoming"; PropertyChanges { target: tabContainerItem; opacity: 1.0; x: 0 } },
                State { name: "Outgoing"; PropertyChanges { target: tabContainerItem; opacity: 0.0; x: -root.width } },
                State { name: "Hidden"; PropertyChanges { target: tabContainerItem; opacity: 0.0; x: root.width } }
            ]

            transitions:  [
                Transition {
                    to: "Incoming"
                    SequentialAnimation {
                        ScriptAction { script: root.clip = true }
                        PropertyAnimation { properties: "opacity,x"; easing.type: Easing.InQuad; duration: 250 }
                        ScriptAction { script: {incomingDone(); root.clip = false} }
                    }
                },
                Transition {
                    to: "Outgoing"
                    SequentialAnimation {
                        PropertyAnimation { properties: "opacity,x"; easing.type: Easing.InQuad; duration: 250 }
                        ScriptAction { script: outgoingDone() }
                    }
                }
            ]
        }
    }

    QtObject {
        id: priv
        property bool reparenting: false
        property bool complete: false
        property Item currentTabContainer: root.currentTab ? root.currentTab.parent : null
        property Item incomingPage
        property Item outgoingPage
        property bool animate: true

        onCurrentTabContainerChanged: {
            for (var i = 0; i < containerHost.children.length; i++) {
                var tabContainer = containerHost.children[i]
                var isNewTab = (tabContainer == currentTabContainer)
                if (isNewTab) {
                    if (tabContainer.state != "") {
                        if (tabContainer.children[0].status != undefined) {
                            incomingPage = tabContainer.children[0]
                            incomingPage.status = PageStatus.Activating // triggers the orientation change
                            incomingPage.visible = true
                            if (incomingPage == outgoingPage)
                                outgoingPage = null
                        }
                        if (animate)
                            tabContainer.state = "Incoming"
                        else
                            tabContainer.incomingDone()
                    }
                } else {
                    if (tabContainer.state != "Hidden") {
                        if (tabContainer.children.length > 0 && tabContainer.children[0].status != undefined) {
                            outgoingPage = tabContainer.children[0]
                            outgoingPage.status = PageStatus.Deactivating
                            if (incomingPage == outgoingPage)
                                incomingPage = null
                        }
                        if (animate)
                            tabContainer.state = "Outgoing"
                        else
                            tabContainer.outgoingDone()
                    }
                }
            }
        }
    }
}
