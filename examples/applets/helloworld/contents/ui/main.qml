/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents

PlasmoidItem {
    Layout.minimumWidth: Kirigami.Units.gridUnit * 5
    Layout.minimumHeight: Kirigami.Units.gridUnit * 5

    implicitHeight: Kirigami.Units.gridUnit * 10
    implicitWidth: Kirigami.Units.gridUnit * 10

    PlasmaComponents.Label {
        anchors.fill: parent
        wrapMode: Text.Wrap
        text: i18n("Hello world")
    }
}
