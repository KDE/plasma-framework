/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import QtQuick.Controls @QQC2_VERSION@
import org.kde.plasma.core 2.0 as PlasmaCore
import "private"

T.Switch {
    id: control
    property real __indicatorMargin: control.indicator && control.indicator.visible && control.indicator.width > 0 ?
        indicator.width + control.spacing : 0

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            implicitIndicatorWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)
    baselineOffset: contentItem.y + contentItem.baselineOffset

    spacing: PlasmaCore.Units.smallSpacing

    hoverEnabled: true

    icon.width: PlasmaCore.Units.iconSizes.sizeForLabels
    icon.height: PlasmaCore.Units.iconSizes.sizeForLabels

    indicator: SwitchIndicator {
        x: !control.mirrored ? control.leftPadding : control.width - width - control.rightPadding
        y: control.topPadding + (control.availableHeight - height) / 2
        control: control
    }

    contentItem: IconLabel {
        leftPadding: control.mirrored ? 0 : control.__indicatorMargin
        rightPadding: !control.mirrored ? 0 : control.__indicatorMargin
        palette: control.palette
        font: control.font
        alignment: Qt.AlignLeft
        display: control.display
        spacing: control.spacing
        iconWidth: control.icon.width
        iconHeight: control.icon.height
        iconSource: control.icon.name || control.icon.source
        labelText: control.text
    }
}
