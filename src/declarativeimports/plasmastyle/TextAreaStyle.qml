/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/


import QtQuick 2.1
import QtQuick.Window 2.2
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle
import QtQuick.Controls.Private 1.0 as QtQuickControlsPrivate
import QtQuick.Controls 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import "private" as Private

/**
 * \internal
 */
QtQuickControlStyle.TextAreaStyle {
    id: style

    ScrollViewStyle {
       id: svs
    }

    font: PlasmaCore.Theme.defaultFont
    backgroundColor: "transparent"
    textColor: control.backgroundVisible ? PlasmaCore.Theme.viewTextColor : PlasmaCore.ColorScope.textColor
    selectionColor: control.backgroundVisible ? PlasmaCore.Theme.viewFocusColor : PlasmaCore.ColorScope.highlightColor
    selectedTextColor: control.backgroundVisible ? PlasmaCore.Theme.viewHighlightedTextColor : PlasmaCore.ColorScope.highlightedTextColor

    // Work around Qt bug where NativeRendering breaks for non-integer scale factors
    // https://bugreports.qt.io/browse/QTBUG-70481
    renderType: QtQuickControlsPrivate.Settings.isMobile || Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering

    frame: PlasmaCore.FrameSvgItem {
        id: base
        anchors.fill: parent
        visible: control.backgroundVisible
        imagePath: "widgets/lineedit"
        prefix: "base"
        Private.TextFieldFocus {
            id: hover
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "hidden")
            anchors.fill: base
        }
        Component.onCompleted: {
            style.padding.left = base.margins.left
            style.padding.top = base.margins.top
            //TODO: if QtControls gets a component for this, use it instead of this hardcoded heuristic
            style.padding.right = base.margins.right + (control.clearButtonShown ? Math.max(control.parent.height*0.8, PlasmaCore.Units.iconSizes.small)+PlasmaCore.Units.smallSpacing : 0)
            style.padding.bottom = base.margins.bottom
        }
    }

    scrollBarBackground: svs.scrollBarBackground
    handle: svs.handle
    incrementControl: svs.incrementControl
    decrementControl: svs.decrementControl

    Component {
        id: editMenuTouch
        EditMenuTouch {}
    }
    Component {
        id: cursorTouch
        CursorDelegate {}
    }

    __cursorHandle: CursorHandleStyle {}
    __cursorDelegate: QtQuickControlsPrivate.Settings.isMobile ? cursorTouch : null
    __selectionHandle: SelectionHandleStyle {}
    property Component __editMenu: QtQuickControlsPrivate.Settings.isMobile ? editMenuTouch : null
}
