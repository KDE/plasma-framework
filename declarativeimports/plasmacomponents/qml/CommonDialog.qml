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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import "." 0.1

Dialog {
    id: root

    property alias titleText: titleAreaText.text
    property url titleIcon
    property variant buttonTexts: []
    property bool privateCloseIcon: false

    signal buttonClicked(int index)

    onButtonTextsChanged: {
        for (var i = buttonRow.children.length; i > 0; --i) {
            buttonRow.children[i - 1].destroy()
        }
        for (var j = 0; j < buttonTexts.length; ++j) {
            var button = buttonComponent.createObject(buttonRow)
            button.text = buttonTexts[j]
            button.index = j
        }
    }

    Component {
        id: buttonComponent
        Button {
            property int index

            onClicked: {
                if (root.status == DialogStatus.Open) {
                    root.buttonClicked(index)
                    root.close()
                }
            }
        }
    }

    QtObject {
        id: internal

        function buttonWidth() {
            switch (buttonTexts.length) {
                case 0: return 0
                case 1: return Math.round((800 - 3 * 4) / 2)
                default: return (buttonContainer.width - (buttonTexts.length + 1) *
                    4) / buttonTexts.length
            }
        }

        function iconSource() {
            return root.titleIcon
        }
    }

    title: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/extender-dragger"
        prefix: "root"
        anchors.left: parent.left
        anchors.right: parent.right
        //FIXME: +5 because of Plasma::Dialog margins
        height: titleAreaText.paintedHeight + margins.top + margins.bottom

        LayoutMirroring.enabled: privateCloseIcon ? false : undefined
        LayoutMirroring.childrenInherit: true

        Item {
            id: titleLayoutHelper // needed to make the text mirror correctly

            anchors {
                left: parent.left
                right: titleAreaIcon.source == "" ? parent.right : titleAreaIcon.left
                top: parent.top
                bottom: parent.bottom
                leftMargin: parent.margins.left
                rightMargin: parent.margins.right
                topMargin: parent.margins.top
                bottomMargin: parent.margins.bottom
            }

            Label {
                id: titleAreaText

                LayoutMirroring.enabled: root.LayoutMirroring.enabled

                anchors.fill: parent

                elide: Text.ElideRight
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        Image {
            id: titleAreaIcon

            anchors.right: parent.right
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
            source: internal.iconSource()
            sourceSize.height: 16
            sourceSize.width: 16

            MouseArea {
                id: iconMouseArea

                property bool pressCancelled

                anchors.centerIn: parent
                width: parent.width + 4
                height: parent.height + 4
                enabled: privateCloseIcon && root.status == DialogStatus.Open

                onPressed: {
                    pressCancelled = false
                }
                onClicked: {
                    if (!pressCancelled)
                        root.reject()
                }
                onExited: pressCancelled = true
            }
        }
    }

    buttons: Item {
        id: buttonContainer

        LayoutMirroring.enabled: false
        LayoutMirroring.childrenInherit: true

        width: parent.width
        height: buttonTexts.length ? 48 + 2 * 2 : 0

        Row {
            id: buttonRow
            objectName: "buttonRow"
            anchors.centerIn: parent
            spacing: 4
        }
    }
}
