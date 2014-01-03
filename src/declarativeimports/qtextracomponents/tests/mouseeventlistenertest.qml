/*
    Copyright 2014 David Edmundson <davidedmundson@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

import org.kde.qtextracomponents 2.0
import QtQuick 2.1
import QtQuick.Layouts 1.0


Item
{
    width: 800
    height: 400
    Row {
        anchors.fill: parent
        MouseEventListener {
            width: 400
            height: 400
            id: mouseListener
            acceptedButtons: Qt.LeftButton
            hoverEnabled: true
            onPressed: {
                updateDebug("Pressed", mouse);
            }
            onPressAndHold: {
                updateDebug("Held", mouse);
            }
            onReleased: {
                mouseState.text = "";
                mousePos.text = "";
                screenPos.text = "";
            }

            function updateDebug(state, mouse) {
                mouseState.text = state
                mousePos.text = mouse.x + "," + mouse.y
                screenPos.text = mouse.screenX + "," + mouse.screenY
            }

            Rectangle {
                color: "red"
                anchors.fill: parent

                //MouseEventListener should still get events, even though this has a mousearea
                MouseArea {
                    anchors.fill: parent
                }
            }
        }

        GridLayout {
            width: 400
            columns: 2
            Text {
                text: "Mouse status:"
            }
            Text {
                id: mouseState
            }
            Text {
                text: "Contains Mouse: "
            }
            Text {
                text: mouseListener.containsMouse
            }
            Text {
                text: "Mouse Position: "
            }
            Text {
                id: mousePos
            }
            Text {
                text: "Screen Position: "
            }
            Text {
                id: screenPos
            }
        }
    }
}