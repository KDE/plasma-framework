/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Templates as T
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents
import org.kde.plasma.plasmoid

PlasmoidItem {
    id: root

    property int currentIndex: 0
    onCurrentIndexChanged: print("AAA", currentIndex)

    fullRepresentation: ColumnLayout {
        spacing: 0

        Layout.minimumWidth: Kirigami.Units.gridUnit * 15
        Layout.minimumHeight: Kirigami.Units.gridUnit * 15

        Layout.maximumWidth: Kirigami.Units.gridUnit * 60
        Layout.maximumHeight: Kirigami.Units.gridUnit * 60

        PlasmaComponents.TabBar {
            id: tabBar

            currentIndex: root.currentIndex

            onCurrentIndexChanged: {
                root.currentIndex = currentIndex;
            }

            Layout.fillWidth: true
            // Layout.preferredHeight:
            // anchors {
            //     left: parent.left
            //     right: parent.right
            //     top: parent.top
            // }

            // height: Kirigami.Units.iconSizes.desktop

            PlasmaComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Theme Page"
                icon.name: "preferences-desktop-appearance"
            }
            PlasmaComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Drag Page"
                icon.name: "preferences-desktop-mouse"
            }
            PlasmaComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Icons Page"
                icon.name: "preferences-desktop-icons"
            }
            PlasmaComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Dialogs Page"
                icon.name: "preferences-system-windows"
            }
            PlasmaComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Buttons Page"
                icon.name: "preferences-desktop-theme"
            }
            PlasmaComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Plasmoid Page"
                icon.name: "plasma"
            }
            PlasmaComponents.TabButton {
                display: T.AbstractButton.IconOnly
                text: "Mouse Page"
                icon.name: "preferences-desktop-mouse"
            }
        }

        PlasmaComponents.SwipeView {
            id: contentViewContainer

            Layout.fillWidth: true
            Layout.fillHeight: true

            clip: true

            currentIndex: root.currentIndex

            onCurrentIndexChanged: {
                root.currentIndex = currentIndex;
            }

            ThemePage {}
            DragPage {}
            // IconsPage {
            //     id: iconsPage
            // }
            // DialogsPage {
            //     id: dialogsPage
            // }
            // ButtonsPage {
            //     id: buttonsPage
            // }
            // PlasmoidPage {
            //     id: plasmoidPage
            // }

            // MousePage {
            //     id: mousePage
            // }

        }
    }
}
