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
import QtQuick.Window 2.2
import QtQuick.Templates @QQC2_VERSION@ as T
import QtQuick.Controls @QQC2_VERSION@ as Controls
import QtGraphicalEffects 1.0
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

T.ComboBox {
    id: control

    implicitWidth: Math.max(background ? background.implicitWidth : 0,
                            contentItem.implicitWidth + leftPadding + rightPadding) + indicator.implicitWidth + rightPadding
    implicitHeight: units.gridUnit * 1.6
    baselineOffset: contentItem.y + contentItem.baselineOffset

    hoverEnabled: true
    topPadding: surfaceNormal.margins.top
    leftPadding: surfaceNormal.margins.left
    rightPadding: surfaceNormal.margins.right
    bottomPadding: surfaceNormal.margins.bottom

    // Work around Qt bug where NativeRendering breaks for non-integer scale factors
    // https://bugreports.qt.io/browse/QTBUG-67007
    renderType: Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering

    delegate: ItemDelegate {
        width: control.popup.width
        text: control.textRole ? (Array.isArray(control.model) ? modelData[control.textRole] : model[control.textRole]) : modelData
        highlighted: control.highlightedIndex == index
        property bool separatorVisible: false
    }

    indicator: PlasmaCore.SvgItem {
        implicitWidth: units.iconSizes.small
        implicitHeight: implicitWidth
        anchors {
            right: parent.right
            rightMargin: surfaceNormal.margins.right
            verticalCenter: parent.verticalCenter
        }
        svg: PlasmaCore.Svg {
            imagePath: "widgets/arrows"
            colorGroup: PlasmaCore.Theme.ButtonColorGroup
        }
        elementId: "down-arrow"
    }

    contentItem: Label {
        text: control.displayText
        font: control.font
        color: theme.buttonTextColor
        horizontalAlignment: Text.AlignLeft
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }

    background: PlasmaCore.FrameSvgItem {
        id: surfaceNormal
        //retrocompatibility with old controls
        implicitWidth: units.gridUnit * 6
        anchors.fill: parent
        imagePath: "widgets/button"
        prefix: control.pressed ? "pressed" : "normal"
        Private.ButtonShadow {
            z: -1
            anchors.fill: parent
            state: {
                if (control.pressed) {
                    return "hidden"
                } else if (control.hovered) {
                    return "hover"
                } else if (control.activeFocus) {
                    return "focus"
                } else {
                    return "shadow"
                }
            }
        }
    }

    popup: T.Popup {
        x: control.mirrored ? control.width - width : 0
        y: control.height
        width: Math.max(control.width, 150)
        implicitHeight: contentItem.implicitHeight
        topMargin: 6
        bottomMargin: 6

        contentItem: ListView {
            id: listview
            clip: true
            implicitHeight: contentHeight
            model: control.popup.visible ? control.delegateModel : null
            currentIndex: control.highlightedIndex
            highlightRangeMode: ListView.ApplyRange
            highlightMoveDuration: 0
            // HACK: When the ComboBox is not inside a top-level Window, it's Popup does not inherit
            // the LayoutMirroring options. This is a workaround to fix this by enforcing
            // the LayoutMirroring options properly.
            // QTBUG: https://bugreports.qt.io/browse/QTBUG-66446
            LayoutMirroring.enabled: Qt.application.layoutDirection === Qt.RightToLeft
            LayoutMirroring.childrenInherit: true
            T.ScrollBar.vertical: Controls.ScrollBar { }
        }
        background: Rectangle {
            anchors {
                fill: parent
                margins: -1
            }
            radius: 2
            color: theme.viewBackgroundColor
            border.color: Qt.rgba(theme.textColor.r, theme.textColor.g, theme.textColor.b, 0.3)
            layer.enabled: true

            layer.effect: DropShadow {
                transparentBorder: true
                radius: 4
                samples: 8
                horizontalOffset: 2
                verticalOffset: 2
                color: Qt.rgba(0, 0, 0, 0.3)
            }
        }
    }
}
