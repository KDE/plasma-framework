/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import QtQuick.Controls 1.0 as QtControls

// for "units"
import org.kde.kirigami 2.20 as Kirigami

Column {
    id: root
    property alias cfg_Speed: slider.value

    Row {
        spacing: Kirigami.Units.largeSpacing

        QtControls.Label {
            width: formAlignment - Kirigami.Units.largeSpacing * 2
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
