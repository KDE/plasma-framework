/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.kirigami 2 as Kirigami

T.DialogButtonBox {
    id: control

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    spacing: Kirigami.Units.smallSpacing
    leftPadding: parent instanceof T.Control ? parent.leftPadding : horizontalPadding
    topPadding: parent instanceof T.Control ? parent.topPadding : verticalPadding
    rightPadding: parent instanceof T.Control ? parent.rightPadding : horizontalPadding
    bottomPadding: parent instanceof T.Control ? parent.bottomPadding : verticalPadding
    alignment: Qt.AlignRight

    delegate: Button {
        width: Math.min(implicitWidth, control.width / control.count - control.rightPadding - control.spacing * (control.count-1))
    }

    contentItem: ListView {
        implicitWidth: contentWidth
        implicitHeight: Kirigami.Units.gridUnit * 1.6

        model: control.contentModel
        spacing: control.spacing
        orientation: ListView.Horizontal
        boundsBehavior: Flickable.StopAtBounds
        snapMode: ListView.SnapToItem
    }

    background: Item {}
}
