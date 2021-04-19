/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Controls @QQC2_VERSION@
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import "." as PC3

T.GroupBox {
    id: control

    implicitWidth: contentWidth + leftPadding + rightPadding
    implicitHeight: contentHeight + topPadding + bottomPadding

    contentWidth: contentItem.implicitWidth || (contentChildren.length === 1 ? contentChildren[0].implicitWidth : 0)
    contentHeight: contentItem.implicitHeight || (contentChildren.length === 1 ? contentChildren[0].implicitHeight : 0)

    padding: 6
    topPadding: padding + (label && label.implicitWidth > 0 ? label.implicitHeight + spacing : 0)

    label: PC3.Label {
        x: control.leftPadding
        width: control.availableWidth

        text: control.title
        font: control.font
        enabled: control.enabled
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
    }

    background: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/frame"
        prefix: "plain"
    }
}
