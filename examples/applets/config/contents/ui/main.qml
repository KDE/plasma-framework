/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts

import org.kde.plasma.plasmoid
import org.kde.plasma.components as PlasmaComponents
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root

    fullRepresentation: ColumnLayout {
        id: column
        spacing: Kirigami.Units.smallSpacing

        Layout.minimumWidth: Kirigami.Units.gridUnit * 10
        Layout.minimumHeight: implicitHeight

        Item {
            Layout.fillHeight: true
        }
        PlasmaComponents.CheckBox {
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            enabled: true
            checked: Plasmoid.configuration.BoolTest
            text: i18n("Bool from config")
            onToggled: {
                Plasmoid.configuration.BoolTest = checked;
            }
        }
        PlasmaComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: i18n("String test")
        }
        PlasmaComponents.TextField {
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            text: Plasmoid.configuration.Test
            onTextEdited: {
                Plasmoid.configuration.Test = text;
            }
        }
        PlasmaComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: i18n("String from another group")
        }
        PlasmaComponents.TextField {
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            text: Plasmoid.configuration.OtherTest
            onTextEdited: {
                Plasmoid.configuration.OtherTest = text;
            }
        }
        PlasmaComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: i18n("Enum\ndisplayed as int,\nwritten as string")
        }
        PlasmaComponents.TextField {
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            text: Plasmoid.configuration.EnumTest
            onTextEdited: {
                Plasmoid.configuration.EnumTest = text;
            }
        }
        PlasmaComponents.Label {
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
            wrapMode: Text.Wrap
            text: i18n("Integer\nminimum: -1\nmaximum: 100")
        }
        PlasmaComponents.SpinBox {
            Layout.fillWidth: true
            Layout.bottomMargin: Kirigami.Units.largeSpacing
            from: -1
            to: 100
            value: Plasmoid.configuration.IntTest
            onValueModified: {
                Plasmoid.configuration.IntTest = value;
            }
        }
        Item {
            Layout.fillHeight: true
        }
    }
}
