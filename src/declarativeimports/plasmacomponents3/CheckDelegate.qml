/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import "private"

T.CheckDelegate {
    id: control

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: Math.max(contentItem.implicitHeight,
                                      indicator ? indicator.implicitHeight : 0) + topPadding + bottomPadding
    hoverEnabled: true

    topPadding: background.margins.top
    bottomPadding: background.margins.bottom
    leftPadding: background.margins.left
    rightPadding: background.margins.right
    spacing: PlasmaCore.Units.smallSpacing

    contentItem: Label {
        leftPadding: control.mirrored ? (control.indicator ? control.indicator.width : 0) + control.spacing : 0
        rightPadding: !control.mirrored ? (control.indicator ? control.indicator.width : 0) + control.spacing : 0

        text: control.text
        font: control.font
        color: PlasmaCore.Theme.viewTextColor
        elide: Text.ElideRight
        visible: control.text
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    indicator: CheckIndicator {
        x: control.mirrored ? control.leftPadding : control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2

        control: control
    }

    background: DefaultListItemBackground {}
}
