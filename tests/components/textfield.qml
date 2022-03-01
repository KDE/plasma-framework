/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 2 TextField"
    property string longText: "This is a longer sentence"

    contentItem: Flow {
        spacing: PlasmaCore.Units.gridUnit

        PlasmaComponents.TextField {
            placeholderText: longText
        }

        PlasmaComponents.TextField {
            text: root.longText
        }

        PlasmaComponents.TextField {
            text: root.longText
            clearButtonShown: true
        }

        PlasmaComponents.TextField {
            width: 400
            placeholderText: longText
        }

        PlasmaComponents.TextField {
            text: root.longText
            echoMode: TextInput.Password
            revealPasswordButtonShown: true
        }

        PlasmaComponents.TextField {
            text: longText
            echoMode: TextInput.Password
            revealPasswordButtonShown: true
            clearButtonShown: true
        }

        PlasmaComponents.TextField {
            text: longText
            LayoutMirroring.enabled: true
            LayoutMirroring.childrenInherit: true
            clearButtonShown: true
        }
    }
}
