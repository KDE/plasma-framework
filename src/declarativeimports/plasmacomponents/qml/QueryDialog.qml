/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

import QtQuick 2.1
import org.kde.plasma.extras 2.0 as PlasmaExtras
import "." 2.0 as PlasmaComponents

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaComponents.CommonDialog {
    id: root
    objectName: "root"

    property string message
    property string acceptButtonText: i18nd("libplasma5", "OK")
    property string rejectButtonText: i18nd("libplasma5", "Cancel")

    onAcceptButtonTextChanged: internal.updateButtonTexts()
    onRejectButtonTextChanged: internal.updateButtonTexts()

    onButtonClicked: {
        print("QueryDialog.buttonClicked");
        if (acceptButtonText && index == 0)
            accept()
        else
            reject()
    }

    content: Item {
        implicitWidth: Math.max(PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 15,  Math.min(label.implicitWidth+12, PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 25))

        implicitHeight: Math.min(PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * 12, label.paintedHeight + 6)


        width: implicitWidth
        height: implicitHeight

        PlasmaExtras.ScrollArea {
            anchors {
                top: parent.top
                topMargin: 6
                bottom: parent.bottom
                left: parent.left
                leftMargin: 6
                right: parent.right
            }

            Flickable {
                id: flickable
                anchors.fill: parent
                contentHeight: label.paintedHeight
                flickableDirection: Flickable.VerticalFlick
                interactive: contentHeight > height

                PlasmaComponents.Label {
                    id: label
                    anchors {
                        top: parent.top
                        right: parent.right
                    }
                    width: flickable.width
                    height: paintedHeight
                    wrapMode: Text.WordWrap
                    text: root.message
                    horizontalAlignment: lineCount > 1 ? Text.AlignLeft : Text.AlignHCenter
                }
            }
        }
    }

    QtObject {
        id: internal

        function updateButtonTexts() {
            var newButtonTexts = []
            if (acceptButtonText)
                newButtonTexts.push(acceptButtonText)
            if (rejectButtonText)
                newButtonTexts.push(rejectButtonText)
            print("+++ new BUtton texts"+newButtonTexts)
            root.buttonTexts = newButtonTexts
        }
    }
}
