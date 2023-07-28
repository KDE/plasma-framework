/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PC3
import org.kde.kirigami 2.20 as Kirigami

Item {
    Layout.minimumWidth: 200
    Layout.minimumHeight: 300

    PlasmaCore.IconItem {
        source: "kbugbuster"
        width: Kirigami.Units.iconSizes.large
        height: width
        anchors {
            right: col.right
            top: col.top
        }
//         opacity: 0.3
    }

    Column {
        id: col

        spacing: Kirigami.Units.largeSpacing
        anchors {
            fill: parent
            margins: Kirigami.Units.gridUnit
        }

        Kirigami.Heading {
            level: 1
            text: i18n("Reporting Bugs")
        }
        Kirigami.Heading {
            level: 3
            text: i18n("So you found something wrong in Plasma...")
            width: parent.width
            wrapMode: Text.Wrap
        }
        PC3.Label {

            width: parent.width
            wrapMode: Text.Wrap

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
