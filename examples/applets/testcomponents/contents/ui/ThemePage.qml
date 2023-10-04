/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

PlasmaComponents.Page {
    id: root

    padding: Kirigami.Units.largeSpacing

    component GridLabel : PlasmaComponents.Label {
        Layout.fillWidth: true
        horizontalAlignment: Text.AlignRight
    }

    component ColorItem : Rectangle {
        implicitWidth: Kirigami.Units.gridUnit * 2
        implicitHeight: Math.round(Kirigami.Units.gridUnit * 1.5)
        radius: 5
        border {
            color: root.Kirigami.Theme.textColor
            width: 1
        }
    }

    contentItem: GridLayout {
        columns: 2
        columnSpacing: Kirigami.Units.largeSpacing
        rowSpacing: Kirigami.Units.largeSpacing

        Kirigami.Heading {
            Layout.columnSpan: 2
            Layout.fillWidth: true
            elide: Text.ElideRight
            level: 1
            text: "Theme"
        }

        PlasmaComponents.Label {
            Layout.columnSpan: 2
            text: "This is the smallest readable Font."
            font: Kirigami.Theme.smallFont
        }

        GridLabel {
            text: "textColor:"
        }
        ColorItem {
            color: Kirigami.Theme.textColor
        }

        GridLabel {
            text: "Button textColor:"
        }
        ColorItem {
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.Button
            color: Kirigami.Theme.textColor
        }

        GridLabel {
            text: "highlightColor:"
        }
        ColorItem {
            color: Kirigami.Theme.highlightColor
        }

        GridLabel {
            text: "View backgroundColor:"
        }
        ColorItem {
            Kirigami.Theme.inherit: false
            Kirigami.Theme.colorSet: Kirigami.Theme.View
            color: Kirigami.Theme.backgroundColor
        }

        Item {
            Layout.fillWidth: true
            Layout.fillHeight: true
        }
    }
}
