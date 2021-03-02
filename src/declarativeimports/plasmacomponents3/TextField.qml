/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Window 2.2
import QtQuick.Controls @QQC2_VERSION@
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kconfig 1.0
import "private" as Private
import org.kde.kirigami 2.5 as Kirigami
import "mobiletextselection" as MobileTextSelection

T.TextField {
    id: control

    /**
     * Whether the button to clear the text from TextField is visible.
     * @since 5.73
     */
    property bool clearButtonShown: false

    /**
     * Whether to show a button that allows the user to reveal the password in
     * plain text. This only makes sense if the echoMode is set to Password.
     * @since 5.73
     */
    property bool revealPasswordButtonShown: false

    // this takes into account kiosk restriction
    readonly property bool __effectiveRevealPasswordButtonShown: revealPasswordButtonShown
                                                              && KAuthorized.authorize("lineedit_reveal_password")
                                                              && (echoMode == TextInput.Normal || control.text.length > 0)

    implicitWidth: Math.max((placeholderText ? placeholder.implicitWidth : 0),
                            units.gridUnit * 8,
                            contentWidth)
                            + base.margins.left + base.margins.right
    implicitHeight: Math.max(units.gridUnit, contentHeight)
                            + topPadding + bottomPadding

    leftPadding: base.margins.left + (LayoutMirroring.enabled ? inlineButtonRow.width : 0)
    topPadding: base.margins.top
    rightPadding: base.margins.right + (LayoutMirroring.enabled ? 0 : inlineButtonRow.width)
    bottomPadding: base.margins.bottom

    color: theme.viewTextColor
    selectionColor: theme.highlightColor
    selectedTextColor: theme.highlightedTextColor
    verticalAlignment: TextInput.AlignVCenter
    opacity: control.enabled ? 1 : 0.6
    hoverEnabled: !Kirigami.Settings.tabletMode

    // Work around Qt bug where NativeRendering breaks for non-integer scale factors
    // https://bugreports.qt.io/browse/QTBUG-70481
    renderType: Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering

    selectByMouse: !Kirigami.Settings.tabletMode

    cursorDelegate: Kirigami.Settings.tabletMode ? mobileCursor : null
    Component {
        id: mobileCursor
        MobileTextSelection.MobileCursor {
            target: control
        }
    }
    onFocusChanged: {
        if (focus) {
            MobileTextSelection.MobileTextActionsToolBar.controlRoot = control;
        }
    }

    onTextChanged: MobileTextSelection.MobileTextActionsToolBar.shouldBeVisible = false;
    onPressed: MobileTextSelection.MobileTextActionsToolBar.shouldBeVisible = true;

    onPressAndHold: {
        if (!Kirigami.Settings.tabletMode) {
            return;
        }
        forceActiveFocus();
        cursorPosition = positionAt(event.x, event.y);
        selectWord();
    }
    MobileTextSelection.MobileCursor {
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
        enabled: false
        horizontalAlignment: control.horizontalAlignment
        verticalAlignment: control.verticalAlignment
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        elide: Text.ElideRight
    }

    Row {
        id: inlineButtonRow
        anchors.right: control.right
        anchors.rightMargin: PlasmaCore.Units.smallSpacing
        anchors.verticalCenter: control.verticalCenter

        PlasmaCore.IconItem {
            id: showPasswordButton
            source: __effectiveRevealPasswordButtonShown ? (control.echoMode === TextInput.Normal ? "visibility": "hint") : ""
            height: Math.max(control.height * 0.8, units.iconSizes.small)
            width: height
            opacity: (__effectiveRevealPasswordButtonShown && control.enabled) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            MouseArea {
                anchors.fill: parent
                enabled: __effectiveRevealPasswordButtonShown
                onClicked: {
                    control.echoMode = (control.echoMode == TextInput.Normal ? TextInput.Password : TextInput.Normal)
                    control.forceActiveFocus()
                }
            }
        }

        PlasmaCore.IconItem {
            id: clearButton
            //ltr confusingly refers to the direction of the arrow in the icon, not the text direction which it should be used in
            source: clearButtonShown ? (LayoutMirroring.enabled ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl") : ""
            height: Math.max(control.height * 0.8, units.iconSizes.small)
            width: height
            opacity: (control.length > 0 && clearButtonShown && control.enabled) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity {
                NumberAnimation {
                    duration: units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    control.text = ""
                    control.forceActiveFocus()
                }
            }
        }
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
