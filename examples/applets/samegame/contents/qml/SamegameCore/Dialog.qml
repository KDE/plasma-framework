/*
    SPDX-FileCopyrightText: 2010 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the QtDeclarative module of the Qt Toolkit.

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1
*/


import Qt 4.7

Rectangle {
    id: page

    property Item text: dialogText

    signal closed
    signal opened
    function forceClose() {
        if(page.opacity == 0)
            return; //already closed
        page.closed();
        page.opacity = 0;
    }

    function show(txt) {
        page.opened();
        dialogText.text = txt;
        page.opacity = 1;
    }

    width: dialogText.width + 20; height: dialogText.height + 20
    color: "white"
    border.width: 1
    opacity: 0
    visible: opacity > 0
    Behavior on opacity {
        NumberAnimation { duration: 1000 }
    }

    Text { id: dialogText; anchors.centerIn: parent; text: "Hello World!" }

    MouseArea { anchors.fill: parent; onClicked: forceClose(); }
}

