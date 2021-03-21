/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

import QtQuick 2.1
import org.kde.kquickcontrolsaddons 2.0
import "AppManager.js" as Utils

import org.kde.plasma.core 2.0 as PlasmaCore

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
        if (event.key == Qt.Key_Right) {
            (priv.mirrored ? priv.goPreviousTab : priv.goNextTab)();
            event.accepted = true
        } else if (event.key == Qt.Key_Left) {
            (priv.mirrored ? priv.goNextTab : priv.goPreviousTab)();
            event.accepted = true
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
            var contentWidth = PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 3
            var contentHeight = PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * 2
            var maxChildSize = 0
            if (childCount != 0) {
                //not too much efficient but the loop over children needs to be done two times to get the proper child width
                for (var i = 0; i < childCount; ++i) {
                    if (!root.children[i].visible || root.children[i].text === undefined) {
                        --visibleChildCount
                    }
                }

                var maxAllowedSize = PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 24
                var itemWidth = Math.min(maxAllowedSize, (root.width - (visibleChildCount-1)*10) / visibleChildCount)
                var itemHeight = Math.min(maxAllowedSize, (root.height - (visibleChildCount-1)*10) / visibleChildCount)

                var itemIndex = mirrored && root.isHorizontal ? childCount - 1 : 0
                var increment = mirrored && root.isHorizontal ? - 1 : 1
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
