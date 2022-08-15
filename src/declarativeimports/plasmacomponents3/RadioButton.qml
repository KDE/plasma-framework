/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.1 as PlasmaCore
import "private"

T.RadioButton {
    id: control

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

    indicator: RadioIndicator {
        x: (control.text || control.icon.name || control.icon.source)
            ? (control.mirrored ? control.width - width - control.rightPadding : control.leftPadding)
            : control.leftPadding + Math.round((control.availableWidth - width) / 2)
        y: control.topPadding + Math.round((control.availableHeight - height) / 2)

        control: control
    }

    contentItem: IconLabel {
        readonly property int effectiveIndicatorWidth: control.indicator && control.indicator.visible && control.indicator.width > 0
            ? control.indicator.width + control.spacing : 0

        mirrored: control.mirrored
        leftPadding: !control.mirrored ? effectiveIndicatorWidth : 0
        rightPadding: control.mirrored ? effectiveIndicatorWidth : 0

        palette: control.palette
        font: control.font
        alignment: Qt.AlignLeft | Qt.AlignVCenter
        display: control.display
        spacing: control.spacing
        iconItem.implicitWidth: control.icon.width
        iconItem.implicitHeight: control.icon.height
        iconItem.source: control.icon.name || control.icon.source
        label.text: control.text

        Rectangle { // As long as we don't enable antialiasing, not rounding should be fine
            parent: control.contentItem.label
            width: Math.min(parent.width, parent.contentWidth)
            height: PlasmaCore.Units.devicePixelRatio
            anchors.left: parent.left
            anchors.top: parent.bottom
            color: PlasmaCore.ColorScope.highlightColor
            visible: control.visualFocus
        }
    }
}
