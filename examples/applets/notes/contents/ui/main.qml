/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.ksvg as KSvg
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents

PlasmoidItem {
    // this isn't a frameSVG, the default SVG margins take up around 7% of the frame size, so we use that
    readonly property real horizontalMargins: width * 0.07
    readonly property real verticalMargins: height * 0.07

    Layout.minimumWidth: Kirigami.Units.gridUnit * 8
    Layout.minimumHeight: Kirigami.Units.gridUnit * 8

    Plasmoid.backgroundHints: PlasmaCore.Types.NoBackground

    onExternalData: (mimetype, data) => {
        if (mimetype === "text/plain") {
            noteText.text = data;
        }
    }

    KSvg.SvgItem {
        anchors.fill: parent

        imagePath: "widgets/notes"
        elementId: "yellow-notes"

        PlasmaComponents.TextArea {
            id: noteText

            anchors {
                fill: parent
                leftMargin: horizontalMargins
                rightMargin: horizontalMargins
                topMargin: verticalMargins
                bottomMargin: verticalMargins
            }

            background: null
            color: Qt.alpha("black", 0.8)
            font.pointSize: Math.round(Kirigami.Theme.defaultFont.pointSize * 1.3)

            text: Plasmoid.configuration.Text
            onEditingFinished: {
                Plasmoid.configuration.Text = text;
            }
        }
    }
}
