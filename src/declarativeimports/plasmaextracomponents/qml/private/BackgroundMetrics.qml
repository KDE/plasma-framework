/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
import QtQuick 2.12
import QtQuick.Window 2.2

import org.kde.plasma.core 2.0 as PlasmaCore

PlasmaCore.FrameSvgItem {
    id: backgroundMetrics
    visible: false
    imagePath: {
        if (Window.window instanceof PlasmaCore.Dialog) {
            return "dialogs/background";
        } else if (plasmoid.formFactor === PlasmaCore.Types.Planar) {
            return "widgets/background";
        // panels and other formfactors are explicitly not supported
        } else {
            return "";
        }
    }
    readonly property bool hasInset: backgroundMetrics.inset.left >= 0 && backgroundMetrics.inset.right >= 0 && backgroundMetrics.inset.top >= 0 && backgroundMetrics.inset.bottom >= 0
}
