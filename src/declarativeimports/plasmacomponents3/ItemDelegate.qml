/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.5
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import "private"

T.ItemDelegate {
    id: control
    property real __indicatorMargin: control.indicator && control.indicator.visible && control.indicator.width > 0 ?
        indicator.width + control.spacing : 0

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            implicitIndicatorWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)
    hoverEnabled: true

    topPadding: background.margins.top
    bottomPadding: background.margins.bottom
    leftPadding: background.margins.left
    rightPadding: background.margins.right
    spacing: PlasmaCore.Units.smallSpacing

    icon.width: PlasmaCore.Units.iconSizes.sizeForLabels
    icon.height: PlasmaCore.Units.iconSizes.sizeForLabels

    contentItem: IconLabel {
        leftPadding: !control.mirrored ? 0 : control.__indicatorMargin
        rightPadding: control.mirrored ? 0 : control.__indicatorMargin
        palette: control.palette
        font: control.font
        alignment: Qt.AlignLeft
        display: control.display
        spacing: control.spacing
        iconItem.implicitWidth: control.icon.width
        iconItem.implicitHeight: control.icon.height
        iconItem.source: control.icon.name || control.icon.source
        label.text: control.text
    }

    background: DefaultListItemBackground {}
}
