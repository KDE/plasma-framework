/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Controls @QQC2_VERSION@
import QtQml.Models 2.1
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami
import "private"

T.TabButton {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            implicitIndicatorWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)
    baselineOffset: contentItem.y + contentItem.baselineOffset

    leftPadding: background.margins.left
    rightPadding: background.margins.right
    topPadding: background.margins.top
    bottomPadding: background.margins.bottom

    spacing: PlasmaCore.Units.smallSpacing

    hoverEnabled: true

    Kirigami.MnemonicData.enabled: control.enabled && control.visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.SecondaryControl
    Kirigami.MnemonicData.label: control.text

    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(control.text))
        sequence: control.Kirigami.MnemonicData.sequence
        onActivated: if (control.action) {
            control.action.trigger()
        } else if (control.checkable && !control.checked) {
            // A checkable AbstractButton clicked by a user would normally
            // change the checked state first before emitting clicked().
            control.toggle()
            // Manually emit clicked() because action.trigger() is the only
            // button related function that automatically emits clicked()
            control.clicked()
        }
    }

    icon.width: PlasmaCore.Units.iconSizes.smallMedium
    icon.height: PlasmaCore.Units.iconSizes.smallMedium

    contentItem: IconLabel {
        palette: control.palette
        font: control.font
        display: control.display
        spacing: control.spacing
        iconItem.implicitWidth: control.icon.width
        iconItem.implicitHeight: control.icon.height
        iconItem.source: control.icon.name || control.icon.source
        iconItem.active: control.visualFocus
        label.text: control.Kirigami.MnemonicData.richTextLabel
        label.color: control.visualFocus ? PlasmaCore.ColorScope.highlightColor : PlasmaCore.ColorScope.textColor
    }

    background: PlasmaCore.FrameSvgItem {
        visible: !control.ListView.view || !control.ListView.view.highlightItem
        imagePath: "widgets/tabbar"
        prefix: control.T.TabBar.position === T.TabBar.Footer ? "south-active-tab" : "north-active-tab"
        enabledBorders: {
            const borders = PlasmaCore.FrameSvgItem.LeftBorder | PlasmaCore.FrameSvgItem.RightBorder
            if (!visible || control.checked) {
                return borders | PlasmaCore.FrameSvgItem.TopBorder | PlasmaCore.FrameSvgItem.BottomBorder
            } else if (control.T.TabBar.position === T.TabBar.Footer) {
                return borders | PlasmaCore.FrameSvgItem.BottomBorder
            } else {
                return borders | PlasmaCore.FrameSvgItem.TopBorder
            }
        }
    }
}
