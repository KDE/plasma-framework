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
import org.kde.plasma.components 0.1


Column {
    id: column

    // for demonstration and testing purposes each component needs to
    // set its inverted state explicitly
    property bool childrenInverted: false
    property bool windowInverted: false

    spacing: 14

    Label {
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Qt Components " + (enabled ? "(enabled)" : "(disabled)")
    }

    Button {
        anchors.horizontalCenter: parent.horizontalCenter
        text: "Push me"
        width: parent.width - parent.spacing
    }

    TextField {
        anchors.horizontalCenter: parent.horizontalCenter
        placeholderText: "TextField"
        width: parent.width - parent.spacing
    }

    TextField {
        id: clearable
        anchors.horizontalCenter: parent.horizontalCenter
        placeholderText: "Clearable TextField"
        text: "Clearable TextField"
        width: parent.width - parent.spacing

        Image {
            id: clearText
            anchors { top: parent.top; right: parent.right; margins: 6 }
            fillMode: Image.PreserveAspectFit
            smooth: true; visible: parent.text
            source: "qrc:close_stop.svg"
            height: parent.height - 6
            width: parent.height - 6

            MouseArea {
                id: clear
                anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter }
                height: clearable.height; width: clearable.height
                onClicked: clearable.text = ""
            }
        }
    }

    TextField {
        id: customOperation
        anchors.horizontalCenter: parent.horizontalCenter
        placeholderText: "Custom operation"
        width: parent.width - parent.spacing

        Image {
            id: addText
            anchors { top: parent.top; right: parent.right }
            smooth: true
            fillMode: Image.PreserveAspectFit
            source: "qrc:ok.svg"
            height: parent.height; width: parent.height
            scale: LayoutMirroring.enabled ? -1 : 1

            MouseArea {
                id: add
                anchors.fill: parent
                onClicked: textSelection.open()
            }

            SelectionDialog {
                id: textSelection
                titleText: "Preset Texts"
                selectedIndex: -1
                model: ListModel {
                    ListElement { name: "Lorem ipsum." }
                    ListElement { name: "Lorem ipsum dolor sit amet." }
                    ListElement { name: "Lorem ipsum dolor sit amet ipsum." }
                }

                onAccepted: {
                    customOperation.text = textSelection.model.get(textSelection.selectedIndex).name
                    customOperation.forceActiveFocus()
                }

                onRejected: selectedIndex = -1
            }
        }
    }

    TextArea {
        anchors.horizontalCenter: parent.horizontalCenter
        placeholderText: "This is a\n multiline control."
        width: parent.width - parent.spacing; height: 280
    }

    Slider {
        anchors.horizontalCenter: parent.horizontalCenter
        value: 50
    }

    ButtonRow {
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: parent.spacing

        exclusive: true

        RadioButton {
        }

        RadioButton {
        }
    }

    Row {
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: parent.spacing

        CheckBox {
        }

        CheckBox {
            checked: true
        }
    }

    Switch {
        anchors.horizontalCenter: parent.horizontalCenter
    }

    ProgressBar {
        anchors.horizontalCenter: parent.horizontalCenter

        Timer {
            running: true
            repeat: true
            interval: 25
            onTriggered: parent.value = (parent.value + 1) % 1.1
        }
    }

    ProgressBar {
        anchors.horizontalCenter: parent.horizontalCenter
        indeterminate: true
    }

    Component {
        id: dialogComponent
        CommonDialog {
            id: dialog
            titleText: "CommonDialog"
            buttonTexts: ["Ok", "Cancel"]

            content: Text {
                text: "This is the content"
                font { bold: true; pixelSize: 16 }
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }
    }

    Button {
        property CommonDialog dialog
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - parent.spacing
        text: "CommonDialog"
        onClicked: {
            if (!dialog)
                dialog = dialogComponent.createObject(column)
            dialog.open()
        }
    }

    Component {
        id: singleSelectionDialogComponent
        SelectionDialog {
            titleText: "Select background color"
            selectedIndex: 1

            model: ListModel {
                id: colorModel

                ListElement { name: "Red" }
                ListElement { name: "Blue" }
                ListElement { name: "Green" }
                ListElement { name: "Yellow" }
                ListElement { name: "Black" }
                ListElement { name: "White" }
                ListElement { name: "Grey" }
                ListElement { name: "Orange" }
                ListElement { name: "Pink" }
            }

            onAccepted: { selectionDialogButton.parent.color = colorModel.get(selectedIndex).name }
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        height: selectionDialogButton.height
        width: parent.width - parent.spacing
        radius: 10

        Button {
            id: selectionDialogButton
            property SelectionDialog singleSelectionDialog
            anchors.centerIn: parent
            text: "Selection Dialog"
            onClicked: {
                if (!singleSelectionDialog)
                    singleSelectionDialog = singleSelectionDialogComponent.createObject(column)
                singleSelectionDialog.open()
            }
        }
    }

    Button {
        property QueryDialog queryDialog
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - parent.spacing
        text: "QueryDialog"
        onClicked: {
            if (!queryDialog)
                queryDialog = queryDialogComponent.createObject(column)
            queryDialog.open()
        }
    }

    Component {
        id: queryDialogComponent
        QueryDialog {
            titleText: "Query Dialog"
            // Arabic character in the beginning to test right-to-left UI alignment
            message: (LayoutMirroring.enabled ? "\u062a" : "") + "Lorem ipsum dolor sit amet, consectetur adipisici elit,"
                     + "sed eiusmod tempor incidunt ut labore et dolore magna aliqua."
                     + "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris"
                     + "nisi ut aliquid ex ea commodi consequat. Quis aute iure reprehenderit"
                     + "in voluptate velit esse cillum dolore eu fugiat nulla pariatur."
                     + "Excepteur sint obcaecat cupiditat non proident, sunt in culpa qui"
                     + "officia deserunt mollit anim id est laborum."

            acceptButtonText: "Ok"
            rejectButtonText: "Cancel"

            titleIcon: "kmail"
        }
    }

    Rectangle {
        anchors.horizontalCenter: parent.horizontalCenter
        height: contentMenuButton.height
        width: parent.width - parent.spacing
        radius: 10

        Button {
            id: contentMenuButton
            property ContextMenu contextMenu
            anchors.horizontalCenter: parent.horizontalCenter
            text: "ContextMenu"
            onClicked: {
                if (!contextMenu)
                    contextMenu = contextMenuComponent.createObject(contentMenuButton)
                contextMenu.open()
            }
        }
    }

    Component {
        id: contextMenuComponent
        ContextMenu {
            MenuItem {
                text: "White"
                onClicked: contentMenuButton.parent.color = "White"
            }
            MenuItem {
                text: "Red"
                onClicked: contentMenuButton.parent.color = "Red"
            }
            MenuItem {
                text: "LightBlue"
                onClicked: contentMenuButton.parent.color = "LightBlue"
            }
            MenuItem {
                text: "LightGreen"
                onClicked: contentMenuButton.parent.color = "LightGreen"
            }
        }
    }

    ListView {
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - parent.spacing; height: 120
        clip: true
        delegate: listDelegate
        model: listModel
        header: listHeading
    }

    ListModel {
        id: listModel

        ListElement {
            titleText: "Title"
            subTitleText: "SubTitle"
        }
        ListElement {
            titleText: "Title2"
            subTitleText: "SubTitle"
        }
        ListElement {
            titleText: "Title3"
            subTitleText: "SubTitle"
        }
    }

    Component {
        id: listHeading
        Label {
            text: "Heading"
        }
    }

    Component {
        id: listDelegate
        ListItem {
            id: listItem
            Column {

                Label {
                    text: titleText
                }
                Label {
                    text: subTitleText
                }
            }
        }
    }

    Label {
        property SelectionDialog selectionDialog
        text: {
            if (selectionDialog) {
                if (selectionDialog.selectedIndex >= 0)
                    return selectionDialog.model.get(selectionDialog.selectedIndex).name
            }
            return "Three"
        }
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - parent.spacing

        MouseArea {
            anchors.fill: parent
            onClicked: {
                if (!selectionDialog)
                    selectionDialog = selectionDialogComponent.createObject(column)
                selectionDialog.open()
            }
        }

        Component {
            id: selectionDialogComponent
            SelectionDialog {
                titleText: "Select"
                selectedIndex: 2
                model: ListModel {
                    ListElement { name: "One" }
                    ListElement { name: "Two" }
                    ListElement { name: "Three" }
                    ListElement { name: "Four" }
                    ListElement { name: "Five" }
                    ListElement { name: "Six" }
                    ListElement { name: "Seven" }
                    ListElement { name: "Eight" }
                    ListElement { name: "Nine" }
                }
            }
        }
    }


    TabBar {
        //width: parent.width - parent.spacing
        //height: 50
        anchors.horizontalCenter: parent.horizontalCenter
        TabButton { tab: tab1content; text: "1"; iconSource: "qrc:close_stop.svg"}
        TabButton { tab: tab2content; text: "2"; iconSource: "konqueror"}
        TabButton { tab: tab3content; text: "3"}
    }

    TabGroup {
        height: 100
        width: parent.width - parent.spacing
        Button { id: tab1content; text: "tab1" }
        Text {
            id: tab2content
            text: "tab2"
            horizontalAlignment: "AlignHCenter"
            verticalAlignment: "AlignVCenter"
        }
        Page {
            id: tab3content
            width: 50
            height: 32
            CheckBox { anchors.fill: parent; text: "tab3"}
        }
    }

    ToolButton {
        id: toolButton
        text: "ToolButton"
        iconSource: "konqueror"
    }

    ToolButton {
        id: toolButton2
        flat: true
        iconSource: "qrc:ok.svg"
    }

    ToolButton {
        id: toolButton3
        text: "ToolButton"
        iconSource: "qrc:close_stop.svg"
    }

    Row {
        spacing: 5

        BusyIndicator {
            id: busyInd1
            width: 20
            height: 20
            running: true
        }

        BusyIndicator {
            // default width/height is 40
            id: busyInd2
            running: true
        }

        BusyIndicator {
            id: busyInd3
            width: 60
            height: 60
            running: true
        }

        Button {
            text: "Toggle"
            onClicked: {
                busyInd1.running = !busyInd1.running
                busyInd2.running = !busyInd2.running
                busyInd3.running = !busyInd3.running
            }
        }
    }

    Button {
        property CommonDialog sectionScroll
        anchors.horizontalCenter: parent.horizontalCenter
        width: parent.width - parent.spacing
        text: "SectionScroller"
        iconSource: "konqueror"
        onClicked: {
            if (!sectionScroll)
                sectionScroll = sectionScrollComponent.createObject(column)
            sectionScroll.open()
        }
    }

    Component {
        id: sectionScrollComponent
        CommonDialog {
            id: sectionScroll
            titleText: "Section Scroller"
            buttonTexts: ["Close"]
            onButtonClicked: close()

            content: Rectangle {
                color: Qr.rgba(1,1,1,0.8)
                width: parent.width
                implicitHeight: 300

                ListModel {
                    id: testModel
                    ListElement { name: "A Cat 1"; alphabet: "A" }
                    ListElement { name: "A Cat 2"; alphabet: "A" }
                    ListElement { name: "Boo 1"; alphabet: "B" }
                    ListElement { name: "Boo 2"; alphabet: "B" }
                    ListElement { name: "Cat 1"; alphabet: "C" }
                    ListElement { name: "Cat 2"; alphabet: "C" }
                    ListElement { name: "Dog 1"; alphabet: "D" }
                    ListElement { name: "Dog 2"; alphabet: "D" }
                    ListElement { name: "Dog 3"; alphabet: "D" }
                    ListElement { name: "Dog 4"; alphabet: "D" }
                    ListElement { name: "Dog 5"; alphabet: "D" }
                    ListElement { name: "Dog 6"; alphabet: "D" }
                    ListElement { name: "Dog 7"; alphabet: "D" }
                    ListElement { name: "Dog 8"; alphabet: "D" }
                    ListElement { name: "Dog 9"; alphabet: "D" }
                    ListElement { name: "Dog 10"; alphabet: "D" }
                    ListElement { name: "Dog 11"; alphabet: "D" }
                    ListElement { name: "Dog 12"; alphabet: "D" }
                    ListElement { name: "Elephant 1"; alphabet: "E" }
                    ListElement { name: "Elephant 2"; alphabet: "E" }
                    ListElement { name: "FElephant 1"; alphabet: "F" }
                    ListElement { name: "FElephant 2"; alphabet: "F" }
                    ListElement { name: "Guinea pig"; alphabet: "G" }
                    ListElement { name: "Goose"; alphabet: "G" }
                    ListElement { name: "Horse"; alphabet: "H" }
                    ListElement { name: "Horse"; alphabet: "H" }
                    ListElement { name: "Parrot"; alphabet: "P" }
                    ListElement { name: "Parrot"; alphabet: "P" }
                }

                ListView {
                    id: list
                    anchors.fill: parent
                    clip: true
                    cacheBuffer: contentHeight
                    delegate:  ListItem {
                        Label {
                            anchors {
                                top: parent.top; topMargin: 4
                                left: parent.left; leftMargin: 4
                            }
                            text: name + " (index " + index + ")"
                            horizontalAlignment: Text.AlignLeft
                        }
                    }

                    model: testModel
                    section.property: "alphabet"
                    section.criteria: ViewSection.FullString
                    section.delegate: ListItem {
                        sectionDelegate: true
                        Label {
                            anchors {
                                top: parent.top; topMargin: 4
                                left: parent.left; leftMargin: 4
                            }
                            text: section
                            horizontalAlignment: Text.AlignLeft
                            font { bold: true; }
                        }
                    }
                }

                SectionScroller {
                    id: sectionScroller
                    listView: list
                }
            }
        }
    }

    ButtonRow {
       id: buttonRow1
       width: parent.width - parent.spacing
       exclusive: true
       checkedButton: b2

       Button { text: "b1" }
       Button { text: "b2" }
       Button { text: "b3" }
   }

   ButtonRow {
       id: buttonRow2
       width: parent.width - parent.spacing
       exclusive: true

       ToolButton { text: "tb1" }
       ToolButton { text: "tb2" }
   }

   ButtonColumn {
       id: buttonColumn
       width: parent.width - parent.spacing
       exclusive: true

       Button { text: "b4" }
       Button { text: "b5" }
       Button { text: "b6" }
       Button { text: "b7" }
   }
}
