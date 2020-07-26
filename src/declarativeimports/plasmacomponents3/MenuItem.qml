/*
 *   Copyright 2016 Marco Martin <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami

T.MenuItem {
    id: controlRoot

    @DISABLE_UNDER_QQC2_2_4@ palette: Kirigami.Theme.palette
    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth + leftPadding + rightPadding + (arrow ? arrow.implicitWidth : 0))
    implicitHeight: Math.max(background ? background.implicitHeight : 0,
                             Math.max(contentItem.implicitHeight,
                                      indicator ? indicator.implicitHeight : 0) + topPadding + bottomPadding)
    baselineOffset: contentItem.y + contentItem.baselineOffset

    leftPadding: highlight.margins.left
    topPadding: highlight.margins.top
    rightPadding: highlight.margins.right
    bottomPadding: highlight.margins.bottom
    spacing: units.smallSpacing
    hoverEnabled: true

    Kirigami.MnemonicData.enabled: controlRoot.enabled && controlRoot.visible
    Kirigami.MnemonicData.controlType: Kirigami.MnemonicData.MenuItem
    Kirigami.MnemonicData.label: controlRoot.text
    Shortcut {
        //in case of explicit & the button manages it by itself
        enabled: !(RegExp(/\&[^\&]/).test(controlRoot.text))
        sequence: controlRoot.Kirigami.MnemonicData.sequence
        onActivated: {
            if (controlRoot.checkable) {
                controlRoot.toggle();
            } else {
                controlRoot.clicked();
            }
        }
    }

    contentItem: RowLayout {
        Item {
           Layout.preferredWidth: (controlRoot.ListView.view && controlRoot.ListView.view.hasCheckables) || controlRoot.checkable ? controlRoot.indicator.width : Kirigami.Units.smallSpacing
        }
        PlasmaCore.IconItem {
            Layout.alignment: Qt.AlignVCenter
            visible: (controlRoot.ListView.view && controlRoot.ListView.view.hasIcons) || (controlRoot.icon != undefined && (controlRoot.icon.name.length > 0 || controlRoot.icon.source.length > 0))
            source: controlRoot.icon ? (controlRoot.icon.name || controlRoot.icon.source) : ""
//FIXME: either migrate to Kirigami icon or add color to IconItem
//            color: controlRoot.icon ? controlRoot.icon.color : "transparent"
            //hovered is for retrocompatibility
            status: (controlRoot.highlighted || controlRoot.hovered) ? PlasmaCore.Svg.Selected : PlasmaCore.Svg.Normal
            Layout.preferredHeight: Math.max(label.height, Kirigami.Units.iconSizes.small)
            Layout.preferredWidth: Layout.preferredHeight
        }
        Label {
            id: label
            Layout.alignment: Qt.AlignVCenter
            Layout.fillWidth: true

            text: controlRoot.Kirigami.MnemonicData.richTextLabel
            font: controlRoot.font
            elide: Text.ElideRight
            visible: controlRoot.text
            horizontalAlignment: Text.AlignLeft
            verticalAlignment: Text.AlignVCenter
        }
    }

//we can't use arrow: on old qqc2 releases
@DISABLE_UNDER_QQC2_2_3@    arrow: PlasmaCore.IconItem {
@DISABLE_UNDER_QQC2_2_3@        x: controlRoot.mirrored ? controlRoot.padding : controlRoot.width - width - controlRoot.padding
@DISABLE_UNDER_QQC2_2_3@        y: controlRoot.topPadding + (controlRoot.availableHeight - height) / 2
@DISABLE_UNDER_QQC2_2_3@        source: controlRoot.mirrored ? "go-next-symbolic-rtl" : "go-next-symbolic"
@DISABLE_UNDER_QQC2_2_3@        status: controlRoot.highlighted ? PlasmaCore.Svg.Selected : PlasmaCore.Svg.Normal
@DISABLE_UNDER_QQC2_2_3@        width: Math.max(label.height, Kirigami.Units.iconSizes.small)
@DISABLE_UNDER_QQC2_2_3@        height: width
@DISABLE_UNDER_QQC2_2_3@        visible: controlRoot.subMenu
@DISABLE_UNDER_QQC2_2_3@    }

    indicator: CheckIndicator {
        x: controlRoot.mirrored ? controlRoot.width - width - controlRoot.rightPadding : controlRoot.leftPadding
        y: controlRoot.topPadding + (controlRoot.availableHeight - height) / 2

        visible: controlRoot.checkable
        control: controlRoot
    }

    background: Item {
        implicitWidth: units.gridUnit * 8

        PlasmaCore.FrameSvgItem {
            id: highlight
            imagePath: "widgets/viewitem"
            prefix: "hover"
            colorGroup: PlasmaCore.ColorScope.colorGroup
            anchors.fill: parent
            opacity: {
                if (controlRoot.highlighted || controlRoot.hovered || controlRoot.pressed) {
                    return 1
                } else {
                    return 0
                }
            }
            Behavior on opacity {
                NumberAnimation {
                    duration: units.shortDuration
                }
            }
        }
    }
}
