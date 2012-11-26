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
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: root

    property alias text: textArea.text

    signal clicked

    property bool checkable: false
    property alias font: textArea.font

    implicitWidth: textArea.paintedWidth + iconItem.width*2 + 6
    implicitHeight: Math.max(theme.smallIconSize, textArea.paintedHeight + 6)
    width: Math.max(implicitWidth, parent.width)

    property bool separator: false
    onSeparatorChanged: {
        if (separator) {
            internal.separatorItem = separatorComponent.createObject(root)
        } else {
            internal.separatorItem.destroy()
        }
    }
    property alias icon: iconItem.source

    enabled: !separator

    PlasmaCore.IconItem {
        id: iconItem
        width: parent.height
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            top: parent.top
            bottom: parent.bottom
        }
    }
    Label {
        id: textArea
        anchors.centerIn: parent

        horizontalAlignment: Text.AlignHCenter
        elide: Text.ElideRight
    }

    QtObject {
        id: internal
        property Item separatorItem
    }
    Component {
        id: separatorComponent
        PlasmaCore.FrameSvgItem {
            imagePath: "widgets/viewitem"
            prefix: "normal"
            height: text ? parent.height : margins.top+margins.bottom
            anchors {
                right: parent.right
                left: parent.left
                verticalCenter: parent.verticalCenter
            }
        }
    }

    MouseArea {
        id: mouseArea

        property bool canceled: false

        anchors.fill: parent

        onPressed: {
            canceled = false
        }
        onClicked: {
            if (!canceled)
                root.clicked()
        }
        onExited: canceled = true
    }

    Keys.onPressed: {
        event.accepted = true
        switch (event.key) {
            case Qt.Key_Select:
            case Qt.Key_Enter:
            case Qt.Key_Return: {
                if (!event.isAutoRepeat) {
                        root.clicked()
                }
                break
            }

            case Qt.Key_Up: {
                    if (ListView.view != null)
                        ListView.view.decrementCurrentIndex()
                    else
                        event.accepted = false
                break
            }

            case Qt.Key_Down: {
                    if (ListView.view != null)
                        ListView.view.incrementCurrentIndex()
                    else
                        event.accepted = false
                break
            }
            default: {
                event.accepted = false
                break
            }
        }
    }
}
