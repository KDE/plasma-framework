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
import org.kde.plasma.core 0.1 as PlasmaCore
import "private" as Private

/**
 * TabBar is a plasma-themed component that you can use as a container for
 * tab buttons.
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

    implicitWidth: layout.implicitWidth + backgroundFrame.margins.left + backgroundFrame.margins.right
    implicitHeight: layout.implicitHeight + backgroundFrame.margins.top + backgroundFrame.margins.bottom

    PlasmaCore.FrameSvgItem {
        id: backgroundFrame

        anchors.fill: parent
        imagePath: "widgets/frame"
        prefix: "sunken"
    }

    Item {
        id: buttonCutter
        anchors {
            fill: parent
            leftMargin: 1
            rightMargin: (buttonsLayout.visible ? buttonsLayout.width : 0) + 1
        }
        clip: true
        PlasmaCore.FrameSvgItem {
            id: buttonFrame

            visible: currentTab !== null
            x: tabBarLayout.x + currentTab.x + backgroundFrame.margins.left -1
            y: backgroundFrame.margins.top
            width: currentTab.width + margins.left + margins.right -1
            height: currentTab.height + margins.top + margins.bottom
            imagePath: "widgets/button"
            prefix: "normal"
            Behavior on x {
                PropertyAnimation {
                    easing.type: Easing.InQuad
                    duration: 250
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
        elementId: "border-left"
        width: naturalSize.width
        visible: buttonsLayout.visible
        anchors {
            left: buttonCutter.left
            leftMargin: -1
            top: buttonCutter.top
            bottom: buttonCutter.bottom
            topMargin: backgroundFrame.margins.top
            bottomMargin: backgroundFrame.margins.bottom
        }
    }
    PlasmaCore.SvgItem {
        svg: scrollWidgetSvg
        elementId: "border-right"
        width: naturalSize.width
        visible: buttonsLayout.visible
        anchors {
            right: buttonCutter.right
            rightMargin: -1
            top: buttonCutter.top
            bottom: buttonCutter.bottom
            topMargin: backgroundFrame.margins.top
            bottomMargin: backgroundFrame.margins.bottom
        }
    }

    onCurrentTabChanged: tabBarLayout.x = Math.max(Math.min(0, -(currentTab.x + currentTab.width/2) + tabbarScroller.width/2), -tabBarLayout.width + tabbarScroller.width)

    onWidthChanged: {
        if (currentTab) {
            tabBarLayout.x = Math.max(Math.min(0, -(currentTab.x + currentTab.width/2) + tabbarScroller.width/2), -tabBarLayout.width + tabbarScroller.width)
        }
    }

    Item {
        id: tabbarScroller
        clip: true
        anchors {
            fill: parent
            leftMargin: backgroundFrame.margins.left + buttonFrame.margins.left
            topMargin: backgroundFrame.margins.top + buttonFrame.margins.top
            rightMargin: backgroundFrame.margins.right + (buttonsLayout.visible ? buttonsLayout.width : buttonFrame.margins.right)
            bottomMargin: backgroundFrame.margins.bottom + buttonFrame.margins.bottom
        }

        Private.TabBarLayout {
            id: tabBarLayout
            //A bit of snap before scolling the layout
            width: (implicitWidth - parent.width < theme.defaultFont.mSize.width*4) ? parent.width : implicitWidth
            anchors {
                top: parent.top
                bottom: parent.bottom
            }
            Behavior on x {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.InOutQuad
                }
            }
        }
    }
    Row {
        id: buttonsLayout
        visible: tabBarLayout.width > root.width - backgroundFrame.margins.left - backgroundFrame.margins.right
        height: Math.min(parent.height, theme.mediumIconSize)

        anchors {
            right: parent.right
            verticalCenter: parent.verticalCenter
            rightMargin: Math.min(y, backgroundFrame.margins.right)
        }
        ToolButton {
            height: parent.height
            width: height
            iconSource: "go-previous"
            enabled: tabBarLayout.x < 0
            onClicked: tabBarLayout.x = Math.min(0, tabBarLayout.x + tabBarLayout.width/tabBarLayout.children.length)
        }
        ToolButton {
            height: parent.height
            width: height
            iconSource: "go-next"
            enabled: tabBarLayout.x > -tabBarLayout.width + tabbarScroller.width
            onClicked: tabBarLayout.x = Math.max(-tabBarLayout.width + tabbarScroller.width, tabBarLayout.x - tabBarLayout.width/tabBarLayout.children.length)
        }
    }
}
