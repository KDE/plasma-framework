/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root

    switchWidth: Kirigami.Units.gridUnit * 10
    switchHeight: Kirigami.Units.gridUnit * 5

    Plasmoid.icon: "tools-report-bug"

    fullRepresentation: PlasmaComponents.ScrollView {
        Layout.minimumWidth: Kirigami.Units.gridUnit * 10
        Layout.preferredWidth: Kirigami.Units.gridUnit * 20

        PlasmaComponents.ScrollBar.horizontal.policy: PlasmaComponents.ScrollBar.AlwaysOff

        contentWidth: availableWidth
        contentHeight: Math.ceil(col.implicitHeight + col.anchors.margins * 2)

        Item {
            width: parent.width

            Kirigami.Icon {
                id: icon

                anchors {
                    top: col.top
                    right: col.right
                }
                width: Kirigami.Units.iconSizes.large
                height: Kirigami.Units.iconSizes.large
                source: root.Plasmoid.icon
            }

            ColumnLayout {
                id: col

                anchors {
                    fill: parent
                    margins: Kirigami.Units.gridUnit
                }
                spacing: Kirigami.Units.largeSpacing

                Kirigami.Heading {
                    level: 1
                    text: i18n("Reporting Bugs")
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    Layout.rightMargin: icon.width
                }
                Kirigami.Heading {
                    level: 3
                    text: i18n("So you found something wrong in Plasma…")
                    wrapMode: Text.Wrap
                    Layout.fillWidth: true
                    Layout.rightMargin: icon.width
                }
                PlasmaComponents.Label {
                    Layout.fillWidth: true
                    wrapMode: Text.Wrap
                    textFormat: Text.StyledText
                    text: i18n("You are running a development version of Plasma. This software is not fit for production use. We do, however encourage testing and reporting the results. A few easy steps to report a bug: <br />\
            <ul>\
            <li>Check <a href=\"\">here if the bug already exists</li>\
            <li>Report it using the form <a href=\"\">here</li>\
            <li>Be ready to provide us feedback, so we can debug the problem</li>\
            </ul>\
            <br />If you would like to participate in development, or have a question, you can ask them on the plasma-devel@kde.org mailing list.\
             ")
                }
            }
        }
    }
}
