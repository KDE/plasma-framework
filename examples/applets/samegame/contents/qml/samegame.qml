/*
    SPDX-FileCopyrightText: 2010 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the QtDeclarative module of the Qt Toolkit.

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1
*/

import Qt 4.7
import "SamegameCore"
import "SamegameCore/samegame.js" as Logic

Rectangle {
    id: screen
    width: 490; height: 720
    property bool inAnotherDemo: false //Samegame often is just plonked straight into other demos
    clip:true

    SystemPalette { id: activePalette }

    Item {
        width: parent.width
        anchors { top: parent.top; left:parent.left; right: parent.right; bottom: toolBar.top }

        Image {
            id: background
            anchors.fill: parent
            source: "SamegameCore/pics/background.png"
            fillMode: Image.Scale
        }

        Item {
            id: gameCanvas
            property int score: 0
            property int blockSize: 40

            z: 20; anchors.centerIn: parent
            width: parent.width - (parent.width % blockSize);
            height: parent.height - (parent.height % blockSize);

            MouseArea {
                anchors.fill: parent; onClicked: Logic.handleClick(mouse.x,mouse.y);
            }
        }
    }

    Dialog { id: dialog; anchors.centerIn: parent; z: 21 }

    Dialog {
        id: nameInputDialog

        property int initialWidth: 0

        anchors.centerIn: parent
        z: 22;

        Behavior on width {
            NumberAnimation {} 
            enabled: nameInputDialog.initialWidth != 0
        }

        onOpened: nameInputText.focus = true;
        onClosed: {
            nameInputText.focus = false;
            if (nameInputText.text != "")
                Logic.saveHighScore(nameInputText.text);
        }
        Text {
            id: dialogText
            anchors { left: nameInputDialog.left; leftMargin: 20; verticalCenter: parent.verticalCenter }
            text: "You won! Please enter your name: "
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (nameInputText.text == "")
                    nameInputText.openSoftwareInputPanel();
                else
                    nameInputDialog.forceClose();
            }
        }

        TextInput {
            id: nameInputText
            anchors { verticalCenter: parent.verticalCenter; left: dialogText.right }
            focus: false
            autoScroll: false
            maximumLength: 24
            onTextChanged: {
                var newWidth = nameInputText.width + dialogText.width + 40;
                if ( (newWidth > nameInputDialog.width && newWidth < screen.width) 
                        || (nameInputDialog.width > nameInputDialog.initialWidth) )
                    nameInputDialog.width = newWidth;
            }
            onAccepted: {
                nameInputDialog.forceClose();
            }
        }
    }

    Rectangle {
        id: toolBar
        width: parent.width; height: 32
        color: activePalette.window
        anchors.bottom: screen.bottom

        Button {
            id: newGameButton
            anchors { left: parent.left; leftMargin: 3; verticalCenter: parent.verticalCenter }
            text: "New Game" 
            onClicked: Logic.startNewGame()
        }

        Button {
            visible: !inAnotherDemo
            text: "Quit"
            anchors { left: newGameButton.right; leftMargin: 3; verticalCenter: parent.verticalCenter }
            onClicked: Qt.quit();
        }

        Text {
            id: score
            anchors { right: parent.right; rightMargin: 3; verticalCenter: parent.verticalCenter }
            text: "Score: " + gameCanvas.score
            font.bold: true
            color: activePalette.windowText
        }
    }
}
