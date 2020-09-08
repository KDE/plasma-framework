/*
 * SPDX-FileCopyrightText: 2020 Aleix Pol Gonzalez <aleixpol@kde.org>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import org.kde.plasma.components 3.0 as PlasmaComponents
//import QtQuick.Controls 2.13 as PlasmaComponents


ComponentBase {
    title: "Plasma Components 3 Idle BusyIndicator"
    contentItem: PlasmaComponents.BusyIndicator {
        running: false
    }
}
