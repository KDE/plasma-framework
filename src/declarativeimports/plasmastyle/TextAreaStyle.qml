/*
 *   Copyright (C) 2014 by Marco MArtin <mart@kde.org>
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
 */


import QtQuick 2.1
import QtQuick.Window 2.2
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle
import QtQuick.Controls.Private 1.0 as QtQuickControlsPrivate
import QtQuick.Controls 1.1

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import "private" as Private

QtQuickControlStyle.TextAreaStyle {
    id: style

    ScrollViewStyle {
       id: svs
    }

    font: theme.defaultFont
    backgroundColor: "transparent"
    textColor: control.backgroundVisible ? theme.viewTextColor : PlasmaCore.ColorScope.textColor
    selectionColor: control.backgroundVisible ? theme.viewFocusColor : PlasmaCore.ColorScope.highlightColor
    selectedTextColor: control.backgroundVisible ? theme.viewHighlightedTextColor : PlasmaCore.ColorScope.highlightedTextColor

    // Work around Qt bug where NativeRendering breaks for non-integer scale factors
    // https://bugreports.qt.io/browse/QTBUG-67007
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
            style.padding.right = base.margins.right + (control.clearButtonShown ? Math.max(control.parent.height*0.8, units.iconSizes.small)+units.smallSpacing : 0)
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
