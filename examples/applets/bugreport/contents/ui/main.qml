/*
 *   Copyright 2014 Sebastian Kügler <sebas@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
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
        width: units.iconSizes.large
        height: width
        anchors {
            right: col.right
            top: col.top
        }
//         opacity: 0.3
    }

    Column {
        id: col

        spacing: units.largeSpacing /2
        anchors {
            fill: parent
            margins: units.largeSpacing
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
