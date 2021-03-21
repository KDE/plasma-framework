/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * \internal
 */
QtQuickControlStyle.ToolBarStyle {

    background: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/toolbar"
        colorGroup: PlasmaCore.ColorScope.colorGroup
    }
}
