/*
 * SPDX-FileCopyrightText: 2020 Marco Martin <notmart@gmail.com>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 3 TabBar with TabButtons"
    contentItem: PlasmaComponents.TabBar {
        implicitWidth: tabButton2.implicitWidth*4
        PlasmaComponents.TabButton {
            icon.name: "application-menu"
            text: "Icon Only"
            display: PlasmaComponents.TabButton.IconOnly
        }
        PlasmaComponents.TabButton {
            id: tabButton2
            icon.name: "application-menu"
            text: "Text Beside Icon"
            display: PlasmaComponents.TabButton.TextBesideIcon
        }
        PlasmaComponents.TabButton {
            icon.name: "application-menu"
            text: "Text Under Icon"
            display: PlasmaComponents.TabButton.TextUnderIcon
        }
        PlasmaComponents.TabButton {
            icon.name: "application-menu"
            text: "Text Only"
            display: PlasmaComponents.TabButton.TextOnly
        }
    }
}

