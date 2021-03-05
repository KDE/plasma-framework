/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import QtQuick.Controls 1.0 as QtControls

// for "units"
import org.kde.plasma.core 2.0 as PlasmaCore

Column {
    id: root
    property alias cfg_Speed: slider.value

    Row {
        spacing: PlasmaCore.Units.largeSpacing / 2

        QtControls.Label {
            width: formAlignment - PlasmaCore.Units.largeSpacing
            horizontalAlignment: Text.AlignRight
            text: "Speed:"
        }
        QtControls.Slider {
            id: slider
            minimumValue: 20
            maximumValue: 150
        }
    }
}
