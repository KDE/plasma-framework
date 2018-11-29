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
import QtQuick.Controls @QQC2_VERSION@
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private
import org.kde.kirigami 2.5 as Kirigami

T.TextField {
    id: control

    implicitWidth: Math.max(units.gridUnit * 8,
                            placeholderText ? placeholder.implicitWidth + leftPadding + rightPadding : 0)
                            || contentWidth + leftPadding + rightPadding
    implicitHeight: units.gridUnit * 1.6

    padding: 6

    color: theme.viewTextColor
    selectionColor: theme.highlightColor
    selectedTextColor: theme.highlightedTextColor
    verticalAlignment: TextInput.AlignVCenter
    opacity: control.enabled ? 1 : 0.6
    hoverEnabled: !Kirigami.Settings.tabletMode

    // Work around Qt bug where NativeRendering breaks for non-integer scale factors
    // https://bugreports.qt.io/browse/QTBUG-67007
    renderType: Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering

    selectByMouse: !Kirigami.Settings.tabletMode

    cursorDelegate: Kirigami.Settings.tabletMode ? mobileCursor : undefined
    Component {
        id: mobileCursor
        Private.MobileCursor {
            target: control
        }
    }
    onFocusChanged: {
        if (focus) {
            Private.MobileTextActionsToolBar.controlRoot = control;
        }
    }

    onPressAndHold: {
        if (!Kirigami.Settings.tabletMode) {
            return;
        }
        forceActiveFocus();
        cursorPosition = positionAt(event.x, event.y);
        selectWord();
    }
    Private.MobileCursor {
        target: control
        selectionStartHandle: true
        property var rect: target.positionToRectangle(target.selectionStart)
        //FIXME: this magic values seem to be always valid, for every font,every dpi, every scaling
        x: rect.x + 5
        y: rect.y + 6
    }

    Label {
        id: placeholder
        x: control.leftPadding
        y: control.topPadding
        width: control.width - (control.leftPadding + control.rightPadding)
        height: control.height - (control.topPadding + control.bottomPadding)

        text: control.placeholderText
        font: control.font
        color: theme.viewTextColor
        opacity: 0.5
        horizontalAlignment: control.horizontalAlignment
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
    }

    background: Item {
        Private.TextFieldFocus {
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "hidden")
            anchors.fill: parent
        }
        PlasmaCore.FrameSvgItem {
            id: base
            anchors.fill: parent
            imagePath: "widgets/lineedit"
            prefix: "base"
        }
    }
}
