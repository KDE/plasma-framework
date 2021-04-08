/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls 1.0
import QtQuick.Controls.Styles 1.1 as QtQuickControlStyle
import QtQuick.Controls.Private 1.0 as QtQuickControlsPrivate

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

import "private" as Private

/**
 * \internal
 */
QtQuickControlStyle.TextFieldStyle {
    id: root

    textColor: control.enabled ? PlasmaCore.Theme.viewTextColor : Qt.rgba(PlasmaCore.Theme.viewTextColor.r, PlasmaCore.Theme.viewTextColor.g, PlasmaCore.Theme.viewTextColor.b, 0.6)
    selectionColor: PlasmaCore.Theme.highlightColor
    selectedTextColor: PlasmaCore.Theme.viewHighlightedTextColor
    placeholderTextColor: Qt.rgba(PlasmaCore.Theme.viewTextColor.r, PlasmaCore.Theme.viewTextColor.g, PlasmaCore.Theme.viewTextColor.b, 0.5)

    /*
     * Since the password echo is a circle without vertical or horizontal lines, it won't be
     * more blurred with different rendring types.
     * Using Qt rendering, the dots will look more aligned and equally spaced.
     * Also if we are on mobile, make sure we use QtRendering
     * Finally, use QtRendering if we're using a non-integer scale factor to work around
     * https://bugreports.qt.io/browse/QTBUG-70481
     *
     * color with an alpha is also broken. https://bugreports.qt.io/browse/QTBUG-70138
     *
     * Given we have so many issues, just enable QtRendering for now
     */
    renderType: Text.QtRendering

    background: Item {
        //QQC button heights are max(backgroundHeight, label + margins).
        //QQC lineedits are only from the background (and if background is not set, just an arbitrary value of 25)
        //Why? I don't know

        //In order to get the same height in both buttons and lineedits we need to apply the same rule here

        implicitHeight: Math.max(metrics.height * 1.6,
                                 metrics.height + base.margins.top + base.margins.bottom)
        implicitWidth: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).width * 12
        opacity: control.enabled ? 1 : 0.6

        TextMetrics {
            id: metrics
            text: "M"
            font: control.font
        }

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

        Component.onCompleted: {
            root.padding.left = base.margins.left
            root.padding.top = base.margins.top
            root.padding.bottom = base.margins.bottom

            //TODO: if QtControls gets a component for this, use it instead of this hardcoded heuristic
            root.padding.right = Qt.binding(function() {
                var actionIconSize = Math.max(control.height * 0.8, PlasmaCore.Units.iconSizes.small);
                //actionCount is an int of the number of items
                var actionCount = (control.hasOwnProperty("clearButtonShown") && control.clearButtonShown) +
                                  (control.hasOwnProperty("__effectiveRevealPasswordButtonShown") && control.__effectiveRevealPasswordButtonShown);
                return base.margins.right + (actionIconSize * actionCount) + (actionCount > 0 ? PlasmaCore.Units.smallSpacing : 0);
            })
        }
    }

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
