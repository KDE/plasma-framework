/*
 *   Copyright 2014 Vishesh Handa <vhanda@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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

import QtQuick 2.0

import QtQuick.Controls 1.1 as Controls
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.Dialog {
    id: root
    location: PlasmaCore.Types.Floating

    Rectangle {
        id: rect
        Layout.minimumWidth: 300
        Layout.minimumHeight: 300

        color: "red"

        Rectangle {
            width: rect.Layout.minimumWidth
            height: rect.Layout.minimumHeight
        }
        ColumnLayout {
            anchors.top: parent.top
            Controls.Label {
                Layout.maximumWidth: rect.Layout.minimumWidth
                text: "Use Alt + Left Click to move the window to a side and then increase the minWidth/Height. The window should reposition itself"
                wrapMode: Text.WordWrap
            }
            Controls.Button {
                text: "Increase MinWidth"
                onClicked: {
                    rect.Layout.minimumWidth = rect.Layout.minimumWidth + 10
                }
            }
            Controls.Button {
                text: "Increase MinHeight"
                onClicked: {
                    rect.Layout.minimumHeight = rect.Layout.minimumHeight + 10
                }
            }
            Controls.Button {
                text: "Increase dialog width"
                onClicked: {
                    root.width = root.width + 10
                }
            }
            Controls.Button {
                text: "Increase dialog height"
                onClicked: {
                    root.height = root.height + 10
                }
            }
        }
    }
}
