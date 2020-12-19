/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

PlasmaCore.SvgItem {
    Layout.minimumWidth: 150
    Layout.minimumHeight: 150
    svg: PlasmaCore.Svg { imagePath: "widgets/notes" }
    elementId: "yellow-notes"

    Connections {
        target: plasmoid
        function onExternalData(mimetype, data) {
            if (mimetype === "text/plain") {
                noteText.text = data
            }
        }
    }

    PlasmaComponents.TextArea {
        id: noteText
        anchors.fill: parent
        anchors.margins: 20
        text: plasmoid.configuration.Text
        onTextChanged: plasmoid.configuration.Text = text
    }
}
