/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.5 as Kirigami

T.MenuItem {
    id: controlRoot

    palette: Kirigami.Theme.palette
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
    spacing: PlasmaCore.Units.smallSpacing
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

    arrow: PlasmaCore.IconItem {
        x: controlRoot.mirrored ? controlRoot.padding : controlRoot.width - width - controlRoot.padding
        y: controlRoot.topPadding + (controlRoot.availableHeight - height) / 2
        source: controlRoot.mirrored ? "go-next-symbolic-rtl" : "go-next-symbolic"
        status: controlRoot.highlighted ? PlasmaCore.Svg.Selected : PlasmaCore.Svg.Normal
        width: Math.max(label.height, Kirigami.Units.iconSizes.small)
        height: width
        visible: controlRoot.subMenu
    }

    indicator: Loader {
        x: controlRoot.mirrored ? controlRoot.width - width - controlRoot.rightPadding : controlRoot.leftPadding
        y: controlRoot.topPadding + Math.round((controlRoot.availableHeight - height) / 2)

        visible: controlRoot.checkable
        sourceComponent: controlRoot.autoExclusive ? radioComponent : checkComponent
    }
    
    Component {
        id: radioComponent
        RadioIndicator {
            control: controlRoot
        }
    }
    Component {
        id: checkComponent
        CheckIndicator {
            control: controlRoot
        }
    }

    background: Item {
        implicitWidth: PlasmaCore.Units.gridUnit * 8

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
                    duration: PlasmaCore.Units.shortDuration
                }
            }
        }
    }
}
