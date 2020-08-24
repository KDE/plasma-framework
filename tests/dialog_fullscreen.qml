/*
    SPDX-FileCopyrightText: 2014 Vishesh Handa <vhanda@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
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
