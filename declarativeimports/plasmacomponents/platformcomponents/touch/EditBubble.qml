/*
*   Copyright 2011 by Sebastian Kügler <sebas@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

PlasmaCore.FrameSvgItem {
    id: editBubble
    objectName: "editBubble"
    property int iconSize: 32;

    imagePath: "dialogs/background"
    width:  4*iconSize
    height: 2*iconSize
    z: 100000
    //anchors { top: parent.bottom; right: parent.right; topMargin: -(iconSize/4); }

    // fully dynamic show / hide
    //state: (textInput.activeFocus && (textInput.selectedText != "" || textInput.canPaste)) ? "expanded" : "collapsed";
    // state controlled externally
    state: "collapsed"

    Row {
        id: buttonRow
        spacing: iconSize
        anchors { horizontalCenter: parent.horizontalCenter; verticalCenter: parent.verticalCenter; margins: 8; }
        height: editBubble.iconSize
        QIconItem {
            id: copyIcon
            icon: QIcon("edit-copy")
            width: editBubble.iconSize
            height: editBubble.iconSize
            enabled: textInput.selectedText != ""
            MouseArea {
                anchors.centerIn: parent
                width: 2*parent.width
                height: 2*parent.height
                onClicked: { textField.copy(); editBubble.state = "collapsed"; }
                onPressed: PropertyAnimation {  target: copyIcon; properties: "scale";
                                                from: 1.0; to: 0.9;
                                                duration: 175; easing.type: Easing.OutExpo; }
                onReleased: PropertyAnimation { target: copyIcon; properties: "scale";
                                                from: 0.9; to: 1.0;
                                                duration: 175; easing.type: Easing.OutExpo; }
            }
        }
        QIconItem {
            id: pasteIcon
            icon: QIcon("edit-paste")
            width: editBubble.iconSize
            height: editBubble.iconSize
            enabled: textInput.canPaste
            MouseArea {
                anchors.centerIn: parent;
                width: 2*parent.width
                height: 2*parent.height
                onClicked: { textField.paste(); editBubble.state = "collapsed"; }
                onPressed: PropertyAnimation {  target: pasteIcon; properties: "scale";
                                                from: 1.0; to: 0.9;
                                                duration: 175; easing.type: Easing.OutExpo; }
                onReleased: PropertyAnimation { target: pasteIcon; properties: "scale";
                                                from: 0.9; to: 1.0;
                                                duration: 175; easing.type: Easing.OutExpo; }
            }
        }
    }
    states: [
        State {
            id: expanded
            name: "expanded";
            PropertyChanges { target: editBubble; opacity: 1.0; scale: 1.0 }
        },
        State {
            id: collapsed
            name: "collapsed";
            PropertyChanges { target: editBubble; opacity: 0; scale: 0.9 }
        }
    ]

    transitions: [
        Transition {
            from: "collapsed"; to: "expanded"
            ParallelAnimation {
                PropertyAnimation { properties: "opacity"; duration: 175; easing.type: Easing.InExpo; }
                PropertyAnimation { properties: "scale"; duration: 175; easing.type: Easing.InExpo; }
            }
        },
        Transition {
            from: "expanded"; to: "collapsed"
            ParallelAnimation {
                PropertyAnimation { properties: "opacity"; duration: 175; easing.type: Easing.OutExpo; }
                PropertyAnimation { properties: "scale"; duration: 100; easing.type: Easing.OutExpo; }
            }
        }
    ]
}
