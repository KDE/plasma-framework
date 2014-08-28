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
    id: dialog
    location: PlasmaCore.Types.Floating

    ColumnLayout {
        Controls.Label {
            Layout.maximumWidth: rect.width
            wrapMode: Text.WordWrap
            text: "Clicking on the rectangle should toggle the full screen mode. Make sure it retains its original geometry when jumping in between full screen and normal"
        }

        Rectangle {
            id: rect
            color: "green"

            width: 500
            height: 500

            MouseArea {
                anchors.fill: parent
                onClicked: {
                    if (dialog.location != PlasmaCore.Types.FullScreen) {
                        dialog.location = PlasmaCore.Types.FullScreen;
                    }
                    else {
                        dialog.location = PlasmaCore.Types.Floating;
                    }
                }
            }
        }
    }
}
