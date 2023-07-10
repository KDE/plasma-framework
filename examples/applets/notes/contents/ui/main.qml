/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.ksvg 1.0 as KSvg
import org.kde.plasma.components 3.0 as PC3

KSvg.SvgItem {
    Layout.minimumWidth: 150
    Layout.minimumHeight: 150
    svg: KSvg.Svg { imagePath: "widgets/notes" }
    elementId: "yellow-notes"

    Connections {
        target: Plasmoid
        function onExternalData(mimetype, data) {
            if (mimetype === "text/plain") {
                noteText.text = data
            }
        }
    }

    PC3.TextArea {
        id: noteText
        anchors.fill: parent
        anchors.margins: 20
        text: Plasmoid.configuration.Text
        onTextChanged: Plasmoid.configuration.Text = text
    }
}
