/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Window 2.2
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle
import QtQuick.Controls.Private 1.0 as QtQuickControlsPrivate
import QtQuick.Layouts 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents

import "private" as Private

/**
 * \internal
 */
QtQuickControlStyle.SpinBoxStyle {
    id: styleRoot

    horizontalAlignment: Qt.AlignRight

    textColor: PlasmaCore.Theme.viewTextColor
    selectionColor: PlasmaCore.Theme.viewFocusColor
    selectedTextColor: PlasmaCore.Theme.viewBackgroundColor


    // Work around Qt bug where NativeRendering breaks for non-integer scale factors
    // https://bugreports.qt.io/browse/QTBUG-70481
    renderType: QtQuickControlsPrivate.Settings.isMobile || Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering

    PlasmaCore.Svg {
        id: arrowSvg
        imagePath: "widgets/arrows"
        colorGroup: PlasmaCore.Theme.ButtonColorGroup
    }

    incrementControl: PlasmaCore.SvgItem {
        anchors {
            fill: parent
            margins : 1
            leftMargin: -1
            rightMargin: 3
        }
        svg: arrowSvg
        elementId: "up-arrow"
        opacity: control.enabled ? (styleData.upPressed ? 1 : 0.6) : 0.5
    }

    decrementControl: PlasmaCore.SvgItem {
        anchors {
            fill: parent
            margins : 1
            leftMargin: -1
            rightMargin: 3
        }
        svg: arrowSvg
        elementId: "down-arrow"
        opacity: control.enabled ? (styleData.downPressed ? 1 : 0.6) : 0.5
    }


    background: Item {
        implicitHeight: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height * 1.6
        implicitWidth: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 12

        Private.TextFieldFocus {
            id: hover
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "hidden")
            anchors.fill: base
        }
        PlasmaCore.FrameSvgItem {
            id: base
            anchors.fill: parent
            imagePath: "widgets/lineedit"
            prefix: "base"
        }
    }
}
