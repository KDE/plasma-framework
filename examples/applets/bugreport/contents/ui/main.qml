/*
    SPDX-FileCopyrightText: 2014 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    Layout.minimumWidth: 200
    Layout.minimumHeight: 300

    PlasmaCore.IconItem {
        source: "kbugbuster"
        width: PlasmaCore.Units.iconSizes.large
        height: width
        anchors {
            right: col.right
            top: col.top
        }
//         opacity: 0.3
    }

    Column {
        id: col

        spacing: PlasmaCore.Units.largeSpacing /2
        anchors {
            fill: parent
            margins: PlasmaCore.Units.largeSpacing
        }

        PlasmaExtras.Heading {
            level: 1
            text: i18n("Reporting Bugs")
        }
        PlasmaExtras.Heading {
            level: 3
            text: i18n("So you found something wrong in Plasma...")
        }
        PlasmaExtras.Paragraph {
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
