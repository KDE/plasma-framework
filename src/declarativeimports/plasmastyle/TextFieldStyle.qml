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

import QtQuick 2.0
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import "private" as Private

QtQuickControlStyle.TextFieldStyle {
    id: root

    textColor: theme.viewTextColor
    selectionColor: theme.viewFocusColor
    selectedTextColor: theme.viewBackgroundColor
    placeholderTextColor: Qt.tint(theme.viewTextColor, "#80000000")

    background: Item {
        implicitHeight: theme.mSize(theme.defaultFont).height * 1.6
        implicitWidth: theme.mSize(theme.defaultFont).width * 12

        Private.TextFieldFocus {
            id: hover
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "hidden")
            anchors.fill: base
        }
        PlasmaCore.FrameSvgItem {
            id: base
            anchors.fill: parent
            // TODO: see what is the correct policy for margins
            //anchors.fill: parent
            imagePath: "widgets/lineedit"
            prefix: "base"
        }
        Component.onCompleted: {
            root.padding.left = base.margins.left
            root.padding.top = base.margins.top
            //TODO: if QtControls gets a component for this, use it instead of this hardcoded heuristic
            root.padding.right = base.margins.right + (control.clearButtonShown ? Math.max(control.parent.height*0.8, units.iconSizes.small)+units.smallSpacing : 0)
            root.padding.bottom = base.margins.bottom
        }
    }
}
