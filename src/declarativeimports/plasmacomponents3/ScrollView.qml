/*
    SPDX-FileCopyrightText: 2017 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2017 The Qt Company Ltd.

    SPDX-License-Identifier: LGPL-3.0-only OR GPL-2.0-or-later
*/


import QtQuick 2.9
import QtQuick.Templates @QQC2_VERSION@ as T
import QtQml 2.15

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.9 as Kirigami
import org.kde.plasma.components 3.0 as PlasmaComponents3

T.ScrollView {
    id: controlRoot

    clip: true

    implicitWidth: Math.max(background ? background.implicitWidth : 0, contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(background ? background.implicitHeight : 0, contentHeight + topPadding + bottomPadding)

    leftPadding: mirrored && T.ScrollBar.vertical.visible && !Kirigami.Settings.isMobile ? T.ScrollBar.vertical.width : 0
    rightPadding: !mirrored && T.ScrollBar.vertical.visible && !Kirigami.Settings.isMobile ? T.ScrollBar.vertical.width : 0
    bottomPadding: T.ScrollBar.horizontal.visible && !Kirigami.Settings.isMobile ? T.ScrollBar.horizontal.height : 0

    data: [
        Kirigami.WheelHandler {
            target: controlRoot.contentItem
            // `Qt.styleHints.wheelScrollLines * 20` is the default scroll speed.
            // ` * PlasmaCore.Units.devicePixelRatio` is needed because Plasma doesn't use Qt scaling on X11.
            horizontalStepSize: Qt.styleHints.wheelScrollLines * 20 * PlasmaCore.Units.devicePixelRatio
            verticalStepSize: Qt.styleHints.wheelScrollLines * 20 * PlasmaCore.Units.devicePixelRatio
        },
        Binding { // TODO KF6: remove, Qt6 has this behavior by default
            target: controlRoot.contentItem // always instanceof Flickable
            property: 'clip'
            value: true
            restoreMode: Binding.RestoreBindingOrValue
        }
    ]

    T.ScrollBar.vertical: PlasmaComponents3.ScrollBar {
        parent: controlRoot
        x: controlRoot.mirrored ? 0 : controlRoot.width - width
        y: controlRoot.topPadding
        height: controlRoot.availableHeight
        active: controlRoot.T.ScrollBar.horizontal.active
    }

    T.ScrollBar.horizontal: PlasmaComponents3.ScrollBar {
        parent: controlRoot
        x: controlRoot.leftPadding
        y: controlRoot.height - height
        width: controlRoot.availableWidth
        active: controlRoot.T.ScrollBar.vertical.active
    }
}
