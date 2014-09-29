/*
 *   Copyright 2014 Vishesh Handa <vhanda@kde.org>
 *   Copyright 2014 Marco Martin <mart@kde.org>
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

Item {
    id: root
    Layout.minimumWidth: 300
    Layout.minimumHeight: 300
    Controls.Button {
        id: button
        anchors.centerIn: parent
        text: "Show Dialog"
        onClicked: {
            //changing the minimumHeight of the mainItem of an hidden dialog
            //shouldn't 
            rect.Layout.minimumHeight = rect.Layout.minimumHeight + 1
            rect.Layout.minimumWidth = rect.Layout.minimumWidth + 1
            subDialog.visible = !subDialog.visible
        }
    }
    PlasmaCore.Dialog {
        id: subDialog
        location: PlasmaCore.Types.Floating
        visualParent: button
        visible: false

        Rectangle {
            id: rect
            Layout.minimumWidth: 300
            Layout.minimumHeight: 300
            Layout.maximumHeight: 500
            property int fixedHeight: 500
            width: 500
            height: fixedHeight

            color: "white"

            Controls.Button {
                text: "Resize"
                anchors.centerIn: parent
                onClicked: {
                    rect.fixedHeight = rect.Layout.minimumHeight = rect.Layout.maximumHeight = (rect.fixedHeight == 500 ? rect.fixedHeight = 100 : rect.fixedHeight = 500)
                }
            }
        }
    }
}
