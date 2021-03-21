/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

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
     * \property list<Item> TabBar::content
     * The content of the TabBarLayout.
     */
    default property alias content: tabBarLayout.data

    /**
     * \property TabBarLayout TabBar::tabBarLayout
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
                    easing.type: Easing.OutCubic
                    duration: PlasmaCore.Units.longDuration
                }
            }
            Behavior on y {
                PropertyAnimation {
                    easing.type: Easing.OutCubic
                    duration: PlasmaCore.Units.longDuration
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

                tabBarLayout.width = (tabBarLayout.implicitWidth - tabbarScroller.width <= PlasmaCore.Units.gridUnit * 4) ? tabbarScroller.width : tabBarLayout.implicitWidth;
            } else {
                tabBarLayout.width = PlasmaCore.Units.gridUnit * 5;
            }
        }
        function syncHeight() {
            if (!layout.isHorizontal) {
                buttonsLayout.width = buttonsLayout.implicitWidth
                buttonsLayout.height = buttonsLayout.implicitHeight

                tabBarLayout.height = (tabBarLayout.implicitHeight - tabbarScroller.height <= PlasmaCore.Units.gridUnit * 4) ? tabbarScroller.height : tabBarLayout.implicitHeight;
            } else {
                tabBarLayout.height = PlasmaCore.Units.gridUnit * 5;
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
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            Behavior on y {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
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
