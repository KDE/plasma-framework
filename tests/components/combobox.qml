/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2014 Marco Martin <notmart@gmail.com>
 * SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0

ComponentBase {
    id: root
    title: "Plasma Components 2 ComboBox"
    contentItem: ColumnLayout {
        spacing: PlasmaCore.Units.largeSpacing
        ListModel {
            id: demoModel
            ListElement { text: "Banana"; color: "Yellow" }
            ListElement { text: "Apple"; color: "Green" }
            ListElement { text: "Coconut"; color: "Brown" }
        }

        ComboBox {
            model:demoModel
        }
        ComboBox {
            editable: true
            model: demoModel
        }
    }
}
