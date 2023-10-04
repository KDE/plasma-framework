/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.extras as PlasmaExtras
import org.kde.plasma.plasmoid

PlasmoidItem {
    id: root

    Plasmoid.title: "Representations Example"

    switchWidth: Kirigami.Units.gridUnit * 10
    switchHeight: Kirigami.Units.gridUnit * 10

    compactRepresentation: MouseArea {
        property bool wasExpanded

        Accessible.name: Plasmoid.title
        Accessible.role: Accessible.Button

        Layout.minimumWidth: Kirigami.Units.gridUnit * 3
        Layout.minimumHeight: Kirigami.Units.gridUnit * 3

        onPressed: wasExpanded = root.expanded
        onClicked: root.expanded = !wasExpanded

        PlasmaComponents.Label {
            anchors.fill: parent
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            wrapMode: Text.Wrap
            text: i18n("Click me")
        }
    }

    fullRepresentation: PlasmaExtras.Representation {
        Layout.preferredWidth: Kirigami.Units.gridUnit * 20
        Layout.preferredHeight: Kirigami.Units.gridUnit * 20

        Layout.minimumWidth: root.switchWidth
        Layout.minimumHeight: root.switchHeight

        header: PlasmaExtras.BasicPlasmoidHeading {}
        contentItem: PlasmaComponents.ScrollView {
            contentWidth: availableWidth
            PlasmaExtras.Heading {
                anchors.fill: parent
                topPadding: Kirigami.Units.gridUnit * 2
                bottomPadding: Kirigami.Units.gridUnit * 2
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
                wrapMode: Text.Wrap
                text: i18n("Hello world")
            }
        }
    }
}
