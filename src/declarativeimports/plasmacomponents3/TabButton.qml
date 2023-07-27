/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2022 ivan (@ratijas) tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls @QQC2_VERSION@
import QtQml.Models 2.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.ksvg 1.0 as KSvg
import org.kde.kirigami 2.5 as Kirigami
import "private" as Private

T.TabButton {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding,
                            implicitIndicatorWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding,
                             implicitIndicatorHeight + topPadding + bottomPadding)

    baselineOffset: contentItem.y + contentItem.baselineOffset
    hoverEnabled: true

    topPadding: background.margins.top
    leftPadding: background.margins.left
    rightPadding: background.margins.right
    bottomPadding: background.margins.bottom
    spacing: Kirigami.Units.smallSpacing

    icon.width: Kirigami.Units.iconSizes.smallMedium
    icon.height: Kirigami.Units.iconSizes.smallMedium

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

    contentItem: Private.IconLabel {
        mirrored: control.mirrored
        palette: control.palette
        font: control.font
        display: control.display
        spacing: control.spacing
        iconItem.implicitWidth: control.icon.width
        iconItem.implicitHeight: control.icon.height
        iconItem.source: control.icon.name || control.icon.source
        iconItem.active: control.visualFocus
        label.text: control.Kirigami.MnemonicData.richTextLabel
        label.color: control.visualFocus ? Kirigami.Theme.highlightColor : Kirigami.Theme.textColor
        Rectangle { // As long as we don't enable antialiasing, not rounding should be fine
            parent: control.contentItem.label
            width: Math.min(parent.width, parent.contentWidth)
            height: 1
            anchors.left: parent.left
            anchors.top: parent.bottom
            color: Kirigami.Theme.highlightColor
            visible: control.visualFocus
        }
    }

    background: KSvg.FrameSvgItem {
        visible: !control.ListView.view || !control.ListView.view.highlightItem
        imagePath: "widgets/tabbar"
        prefix: control.T.TabBar.position === T.TabBar.Footer ? "south-active-tab" : "north-active-tab"
        enabledBorders: {
            const borders = KSvg.FrameSvgItem.LeftBorder | KSvg.FrameSvgItem.RightBorder
            if (!visible || control.checked) {
                return borders | KSvg.FrameSvgItem.TopBorder | KSvg.FrameSvgItem.BottomBorder
            } else if (control.T.TabBar.position === T.TabBar.Footer) {
                return borders | KSvg.FrameSvgItem.BottomBorder
            } else {
                return borders | KSvg.FrameSvgItem.TopBorder
            }
        }
    }
}
