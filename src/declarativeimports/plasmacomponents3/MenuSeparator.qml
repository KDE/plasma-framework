/*
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.
    SPDX-FileCopyrightText: 2019 Alexander Stippich <a.stippich@gmx.net>
    SPDX-FileCopyrightText: 2021 Noah Davis <noahadvs@gmail.com>

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick 2.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.MenuSeparator {
    id: controlRoot
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)
    verticalPadding: Math.round(PlasmaCore.Units.smallSpacing/2)
    hoverEnabled: false
    focusPolicy: Qt.NoFocus
    contentItem: Rectangle {
        // same as MenuItem background
        implicitWidth: PlasmaCore.Units.gridUnit * 8
        implicitHeight: PlasmaCore.Units.devicePixelRatio
        color: PlasmaCore.ColorScope.textColor
        opacity: 0.2
    }
}
