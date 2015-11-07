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

/**Documented API
Inherits:
        Item

Imports:
        QtQuick 2.1

Description:
 TODO

Properties:

**/

import QtQuick 2.1
import org.kde.kquickcontrolsaddons 2.0
import "AppManager.js" as Utils

Item {
    id: root

    property int tabPosition: Qt.TopEdge

    property bool isHorizontal: (tabPosition != Qt.LeftEdge && tabPosition != Qt.RightEdge)

    property int minimumWidth: 0
    property int minimumHeight: 0

    Component.onCompleted: layoutTimer.restart()
    onChildrenChanged: layoutTimer.restart()
    onWidthChanged: layoutTimer.restart()
    onHeightChanged: layoutTimer.restart()

    Keys.onPressed: {
        if (event.key == Qt.Key_Right || event.key == Qt.Key_Left) {
            if (event.key == Qt.Key_Right || priv.mirrored) {
                priv.goNextTab()
                event.accepted = true
            } else if (event.key == Qt.Key_Left || priv.mirrored) {
                priv.goPreviousTab()
                event.accepted = true
            }
        }
    }

    focus: true

    Timer {
        id: layoutTimer
        interval: 10
        onTriggered: priv.layoutChildren()
    }

    MouseEventListener {
        anchors.fill: parent
        onWheelMoved: {
            if (wheel.delta < 0) {
                goNextTab()
            } else {
                goPreviousTab()
            }
        }

        id: priv
        property Item firstButton: root.children.length > 1 ? root.children[1] : null
        property Item firstTab: firstButton ? (firstButton.tab != null ? firstButton.tab : null) : null
        property Item tabGroup: firstTab ? Utils.findParent(firstTab, "currentTab") : null
        property bool mirrored: root.LayoutMirroring.enabled
        property Item tabBar: Utils.findParent(root, "currentTab")

        onMirroredChanged: layoutChildren()

        function goNextTab() {
            var oldIndex = priv.currentButtonIndex();
            while (oldIndex < root.children.length) {
                ++oldIndex

                if (oldIndex > root.children.length - 1) {
                    oldIndex = 1
                }

                //anything with a checked property may act as tabbutton
                if (root.children[oldIndex].checked === undefined) {
                    continue
                }


                if (root.children[oldIndex].visible) {
                    priv.setCurrentButtonIndex(oldIndex)
                    break
                }
            }

            if (root.children[oldIndex]) {
                root.children[oldIndex].clicked();
            }
        }

        function goPreviousTab() {
            var oldIndex = priv.currentButtonIndex();
            while (oldIndex > 0) {
                --oldIndex

                if (oldIndex <= 0) {
                    oldIndex = root.children.length - 1
                }

                //anything with a checked property may act as tabbutton
                if (root.children[oldIndex].checked === undefined) {
                    continue
                }

                if (root.children[oldIndex].visible) {
                    priv.setCurrentButtonIndex(oldIndex)
                    break
                }
            }

            if (root.children[oldIndex]) {
                root.children[oldIndex].clicked();
            }
        }

        function currentButtonIndex() {
            for (var i = 0; i < root.children.length; ++i) {
                if (root.children[i] == priv.tabBar.currentTab)
                    return i
            }
            return -1
        }

        function setCurrentButtonIndex(index) {
            if (tabGroup) {
                tabGroup.currentTab = root.children[index].tab
            }

            priv.tabBar.currentTab = root.children[index]
        }

        function layoutChildren() {
            priv.tabBar = Utils.findParent(root, "currentTab")
            var childCount = root.children.length
            var visibleChildCount = childCount
            var contentWidth = theme.mSize(theme.defaultFont).width * 3
            var contentHeight = theme.mSize(theme.defaultFont).height * 2
            var maxChildSize = 0
            if (childCount != 0) {
                //not too much efficient but the loop over children needs to be done two times to get the proper child width
                for (var i = 0; i < childCount; ++i) {
                    if (!root.children[i].visible || root.children[i].text === undefined) {
                        --visibleChildCount
                    }
                }

                var maxAllowedSize = theme.mSize(theme.defaultFont).width * 24
                var itemWidth = Math.min(maxAllowedSize, (root.width - (visibleChildCount-1)*10) / visibleChildCount)
                var itemHeight = Math.min(maxAllowedSize, (root.height - (visibleChildCount-1)*10) / visibleChildCount)

                var itemIndex = mirrored ? childCount - 1 : 0
                var increment = mirrored ? - 1 : 1
                var visibleIndex = 0

                for (var i = 0; i < childCount; ++i, itemIndex += increment) {
                    var child = root.children[itemIndex]
                    if (!child.visible || root.children[itemIndex].text === undefined) {
                        continue
                    }

                    //Vertical
                    if (!root.isHorizontal) {
                        child.x = 0
                        child.y = visibleIndex * itemHeight + visibleIndex*10
                        ++visibleIndex
                        child.width = root.width
                        child.height = itemHeight

                        if (child.implicitHeight != undefined) {
                            maxChildSize = Math.max(maxChildSize, Math.min(maxAllowedSize, child.implicitHeight));

                            contentWidth = Math.max(contentWidth, (child.implicitWidth + buttonFrame.margins.left + buttonFrame.margins.right));

                            contentHeight = Math.max(contentHeight, maxChildSize  * childCount);
                        }

                    //Horizontal
                    } else {
                        child.x = visibleIndex * itemWidth + visibleIndex*10
                        ++visibleIndex
                        child.y = 0
                        child.width = itemWidth
                        child.height = root.height

                        if (child.implicitWidth != undefined) {
                            maxChildSize = Math.max(maxChildSize, Math.min(maxAllowedSize, child.implicitWidth))
                            contentWidth = Math.max(contentWidth, maxChildSize * childCount)
                            contentHeight = Math.max(contentHeight, (child.implicitHeight + buttonFrame.margins.top + buttonFrame.margins.bottom))
                        }
                    }
                }
            }
            root.implicitWidth = contentWidth
            root.implicitHeight = contentHeight
            root.minimumWidth = itemWidth * visibleChildCount
            root.minimumHeight = itemHeight * visibleChildCount
            if ( priv.tabBar.currentTab === null) {
                //99% of the cases this loop will be length 1 but a tabbar can also have other children, such as Repeater
                for (var i = 0; i < tabBarLayout.children.length; ++i) {
                    //anything with a checked property may act as tabbutton
                    if (tabBarLayout.children[i].checked !== undefined) {
                        priv.tabBar.currentTab = tabBarLayout.children[i]
                        break;
                    }
                }
            }
        }
    }
}
