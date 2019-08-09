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

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

/**
 * TabBar is a plasma-themed component that you can use as a container for
 * tab buttons.
 *
 * @inherit QtQuick.Item
 */
FocusScope {
    id: root
    /**
     * type:list<Item>
     * The content of the TabBarLayout.
     */
    default property alias content: tabBarLayout.data

    /**
     * type:TabBarLayout
     * The layout of the TabBar.
     */
    property alias layout: tabBarLayout

    //Plasma extension
    /**
     * The current tabbar button.
     */
    property Item currentTab

    /**
     * The position of the tabs regarding the contents.
     * possibilities:
     * Qt.TopEdge (default)
     * Qt.LeftEdge
     * Qt.RightEdge
     * Qt.BottomEdge
     */
    property alias tabPosition: tabBarLayout.tabPosition

    implicitWidth: layout.implicitWidth
    implicitHeight: layout.implicitHeight

    Layout.minimumWidth: tabBarLayout.minimumWidth
    Layout.minimumHeight: tabBarLayout.minimumHeight

    Accessible.role: Accessible.PageTabList

    Item {
        id: buttonCutter
        anchors.fill: parent
        clip: true

        PlasmaCore.FrameSvgItem {
            id: buttonFrame

            visible: currentTab && currentTab !== null
            x: currentTab && layout.isHorizontal ? tabBarLayout.x + currentTab.x : 0
            y: !currentTab || layout.isHorizontal ? 0 : tabBarLayout.y + currentTab.y
            width: currentTab && layout.isHorizontal ? currentTab.width + margins.left + margins.right -1 : parent.width
            height: !currentTab || layout.isHorizontal ? parent.height : currentTab.height + margins.top + margins.bottom
            imagePath: "widgets/tabbar"
            prefix: {
                var prefix;
                switch (tabPosition) {
                case Qt.LeftEdge:
                    prefix = "west-active-tab";
                    break;
                case Qt.TopEdge:
                    prefix = "north-active-tab";
                    break;
                case Qt.RightEdge:
                    prefix = "east-active-tab";
                    break;
                default:
                    prefix = "south-active-tab";
                }
                if (!hasElementPrefix(prefix)) {
                    prefix = "active-tab";
                }
                return prefix;
            }
            Behavior on x {
                PropertyAnimation {
                    easing.type: Easing.InQuad
                    duration: units.longDuration
                }
            }
            Behavior on y {
                PropertyAnimation {
                    easing.type: Easing.InQuad
                    duration: units.longDuration
                }
            }
        }
    }

    PlasmaCore.Svg {
        id: scrollWidgetSvg
        imagePath: "widgets/scrollwidget"
    }
    PlasmaCore.SvgItem {
        svg: scrollWidgetSvg
        elementId: layout.isHorizontal ? "border-left" : "border-top"
        width: naturalSize.width
        height: naturalSize.height
        visible: buttonsLayout.visible
        anchors {
            margins: -1
            left: buttonCutter.left
            right: layout.isHorizontal ? undefined : buttonCutter.right
            top: buttonCutter.top
            bottom: layout.isHorizontal ? buttonCutter.bottom : undefined
        }
    }
    PlasmaCore.SvgItem {
        svg: scrollWidgetSvg
        elementId: layout.isHorizontal ? "border-right" : "border-bottom"
        width: naturalSize.width
        height: naturalSize.height
        visible: buttonsLayout.visible
        anchors {
            margins: -1
            left: layout.isHorizontal ? undefined : buttonCutter.left
            right: buttonCutter.right
            top: layout.isHorizontal ? buttonCutter.top : undefined
            bottom: buttonCutter.bottom
        }
    }

    onCurrentTabChanged: updateTabPosition()
    onWidthChanged: updateTabPosition()
    onHeightChanged: updateTabPosition()

    function updateTabPosition() {
        if (!currentTab) {
            return;
        }
        if (layout.isHorizontal) {
            tabBarLayout.x = Math.max(Math.min(0, -(currentTab.x + currentTab.width/2) + tabbarScroller.width/2), -tabBarLayout.width + tabbarScroller.width);
        } else {
            tabBarLayout.y = Math.max(Math.min(0, -(currentTab.y + currentTab.height/2) + tabbarScroller.height/2), -tabBarLayout.height + tabbarScroller.height);
        }
    }

    Item {
        id: tabbarScroller
        clip: true

        anchors {
            fill: parent
            leftMargin: buttonFrame.margins.left
            topMargin: buttonFrame.margins.top
            rightMargin: buttonsLayout.visible && layout.isHorizontal ? buttonsLayout.width : buttonFrame.margins.right
            bottomMargin: buttonsLayout.visible && !layout.isHorizontal ? buttonsLayout.height : buttonFrame.margins.bottom
        }

        function syncWidth() {
            if (layout.isHorizontal) {
                buttonsLayout.width = buttonsLayout.implicitWidth
                buttonsLayout.height = buttonsLayout.implicitHeight

                tabBarLayout.width = (tabBarLayout.implicitWidth - tabbarScroller.width <= units.gridUnit * 4) ? tabbarScroller.width : tabBarLayout.implicitWidth;
            } else {
                tabBarLayout.width = units.gridUnit * 5;
            }
        }
        function syncHeight() {
            if (!layout.isHorizontal) {
                buttonsLayout.width = buttonsLayout.implicitWidth
                buttonsLayout.height = buttonsLayout.implicitHeight

                tabBarLayout.height = (tabBarLayout.implicitHeight - tabbarScroller.height <= units.gridUnit * 4) ? tabbarScroller.height : tabBarLayout.implicitHeight;
            } else {
                tabBarLayout.height = units.gridUnit * 5;
            }
        }

        onWidthChanged: syncWidth();
        onHeightChanged: syncHeight();

        Component.onCompleted: {
            tabbarScroller.syncWidth();
            tabbarScroller.syncHeight();
        }

        Private.TabBarLayout {
            id: tabBarLayout
            //A bit of snap before scrolling the layout

            onImplicitWidthChanged: tabbarScroller.syncWidth();
            onImplicitHeightChanged: tabbarScroller.syncHeight();

            anchors {
                top: layout.isHorizontal ? parent.top : undefined
                bottom: layout.isHorizontal ? parent.bottom : undefined
                left: !layout.isHorizontal ? parent.left : undefined
                right: !layout.isHorizontal ? parent.right : undefined
            }

            Behavior on x {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            Behavior on y {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }

    GridLayout {
        id: buttonsLayout
        rows: 1
        columns: 1
        flow: layout.isHorizontal ? GridLayout.TopToBottom : GridLayout.LeftToRight
        visible: layout.isHorizontal ? tabBarLayout.minimumWidth >= root.width : tabBarLayout.height >= root.minimumHeight

        anchors {
            right: layout.isHorizontal ? parent.right : undefined
            verticalCenter: layout.isHorizontal ? parent.verticalCenter : undefined
            bottom: !layout.isHorizontal ? parent.bottom : undefined
            horizontalCenter: !layout.isHorizontal ? parent.horizontalCenter : undefined
        }
        ToolButton {
            height: Math.min(parent.height, parent.width)
            width: height
            iconSource: layout.isHorizontal ? "go-previous" : "go-up"
            enabled: layout.isHorizontal ? tabBarLayout.x < 0 : tabBarLayout.y < 0
            onClicked: {
                if (layout.isHorizontal) {
                    tabBarLayout.x = Math.min(0, tabBarLayout.x + tabBarLayout.width/tabBarLayout.children.length);
                } else {
                    tabBarLayout.y = Math.min(0, tabBarLayout.y + tabBarLayout.height/tabBarLayout.children.length);
                }
            }
        }
        ToolButton {
            height: Math.min(parent.height, parent.width)
            width: height
            iconSource: layout.isHorizontal ? "go-next" : "go-down"
            enabled: layout.isHorizontal ? tabBarLayout.x > -tabBarLayout.width + tabbarScroller.width : tabBarLayout.y > -tabBarLayout.height + tabbarScroller.height
            onClicked: {
                if (layout.isHorizontal) {
                    tabBarLayout.x = Math.max(-tabBarLayout.width + tabbarScroller.width, tabBarLayout.x - tabBarLayout.width/tabBarLayout.children.length);
                } else {
                    tabBarLayout.y = Math.max(-tabBarLayout.height + tabbarScroller.height, tabBarLayout.y - tabBarLayout.height/tabBarLayout.children.length);
                }
            }
        }
    }
}
