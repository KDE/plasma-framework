/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
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
     * @deprecated since 5.93 Use SearchField instead
     */
    property bool clearButtonShown: false

    /**
     * Whether to show a button that allows the user to reveal the password in
     * plain text. This only makes sense if the echoMode is set to Password.
     * @since 5.73
     * @deprecated since 5.93 Use PasswordField instead
     */
    property bool revealPasswordButtonShown: false

    // this takes into account kiosk restriction
    readonly property bool __effectiveRevealPasswordButtonShown: revealPasswordButtonShown
                                                              && KAuthorized.authorize("lineedit_reveal_password")
                                                              && (echoMode == TextInput.Normal || control.text.length > 0)

    // Can't guarantee that background will always be present or have the margins property
    readonly property bool __hasBackgroundAndMargins: background && background.hasOwnProperty("margins")

    // store information that echoMode was set to Password, regardless of its current value
    property var __isPassword: false
    onEchoModeChanged: __isPassword |= (echoMode === TextInput.Password);

    // TextField doesn't have this property by default for whatever reason
    property bool visualFocus: control.activeFocus && (
        control.focusReason == Qt.TabFocusReason ||
        control.focusReason == Qt.BacktabFocusReason ||
        control.focusReason == Qt.ShortcutFocusReason
    )

    /* It might be preferable to do background width OR content width if we
     * want content to stay within the background rather than expanding the
     * control, but this is maintaining compatibility with the pre-existing
     * behavior. Use the following 2 lines if you want text to stay within the
     * background:
    implicitBackgroundWidth + leftInset + rightInset
    || Math.ceil(Math.max(contentWidth + leftPadding + rightPadding, placeholder.implicitWidth))
     */
    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            Math.ceil(Math.max(contentWidth + leftPadding + rightPadding, placeholder.implicitWidth)))
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             Math.max(contentHeight + topPadding + bottomPadding, placeholder.implicitHeight, __isPassword ? passwordsizeholder.implicitHeight : 0))

    topPadding: __hasBackgroundAndMargins ? background.margins.top : 0
    leftPadding: (__hasBackgroundAndMargins ? background.margins.left : 0) + (control.effectiveHorizontalAlignment === TextInput.AlignRight ? inlineButtonRow.width : 0)
    rightPadding: (__hasBackgroundAndMargins ? background.margins.right : 0) + (control.effectiveHorizontalAlignment === TextInput.AlignRight ? 0 : inlineButtonRow.width)
    bottomPadding: __hasBackgroundAndMargins ? background.margins.bottom : 0

    PlasmaCore.ColorScope.inherit: !background || !background.visible
    PlasmaCore.ColorScope.colorGroup: PlasmaCore.Theme.ViewColorGroup

    color: control.backgroundVisible ? PlasmaCore.Theme.viewTextColor : PlasmaCore.ColorScope.textColor
    selectionColor: control.backgroundVisible ? PlasmaCore.Theme.viewFocusColor : PlasmaCore.ColorScope.highlightColor
    selectedTextColor: control.backgroundVisible ? PlasmaCore.Theme.viewHighlightedTextColor : PlasmaCore.ColorScope.highlightedTextColor
    placeholderTextColor: PlasmaCore.Theme.disabledTextColor

    verticalAlignment: TextInput.AlignVCenter
    // Manually setting this fixes alignment in RTL layouts
    horizontalAlignment: TextInput.AlignLeft
    opacity: control.enabled ? 1 : 0.6
    hoverEnabled: !Kirigami.Settings.tabletMode

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
        enabled: false
        x: 0
        y: 0
        topPadding: control.topPadding
        bottomPadding: control.bottomPadding
        leftPadding: control.leftPadding
        rightPadding: control.rightPadding
        height: control.height
        width: control.width
        font: control.font
        LayoutMirroring.enabled: false
        horizontalAlignment: control.effectiveHorizontalAlignment
        verticalAlignment: control.verticalAlignment
        elide: Text.ElideRight
        renderType: control.renderType
        text: control.placeholderText
        visible: !control.length && !control.preeditText && (!control.activeFocus || control.horizontalAlignment !== Qt.AlignHCenter)
        color: control.placeholderTextColor
    }

    // Object holding the size (implicitHeight) of the password dot character,
    // so that a password TextField never gets shorter than required to display it
    Label {
        id: passwordsizeholder
        enabled: false
        visible: false
        topPadding: control.topPadding
        bottomPadding: control.bottomPadding
        leftPadding: control.leftPadding
        rightPadding: control.rightPadding
        font: control.font
        horizontalAlignment: control.horizontalAlignment
        verticalAlignment: control.verticalAlignment
        elide: Text.ElideRight
        renderType: control.renderType
        text: control.passwordCharacter
    }

    Row {
        id: inlineButtonRow
        anchors.right: control.right
        anchors.rightMargin: control.__hasBackgroundAndMargins ? background.margins.right : 0
        anchors.verticalCenter: control.verticalCenter
        LayoutMirroring.enabled: control.effectiveHorizontalAlignment === TextInput.AlignRight

        PlasmaCore.IconItem {
            id: showPasswordButton
            source: __effectiveRevealPasswordButtonShown ? (control.echoMode === TextInput.Normal ? "visibility": "hint") : ""
            height: PlasmaCore.Units.iconSizes.small
            width: height
            opacity: (__effectiveRevealPasswordButtonShown && control.enabled) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
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
            source: clearButtonShown ? (control.effectiveHorizontalAlignment === TextInput.AlignRight ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl") : ""
            height: PlasmaCore.Units.iconSizes.small
            width: height
            opacity: (control.length > 0 && clearButtonShown && control.enabled) ? 1 : 0
            visible: opacity > 0
            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.InOutQuad
                }
            }
            MouseArea {
                anchors.fill: parent
                onClicked: {
                    control.clear()
                    control.forceActiveFocus()
                }
            }
        }
    }

    background: PlasmaCore.FrameSvgItem {
        implicitWidth: PlasmaCore.Units.gridUnit * 8 + margins.left + margins.right
        implicitHeight: PlasmaCore.Units.gridUnit + margins.top + margins.bottom
        imagePath: "widgets/lineedit"
        prefix: "base"

        PlasmaCore.FrameSvgItem {
            anchors {
                fill: parent
                leftMargin: -margins.left
                topMargin: -margins.top
                rightMargin: -margins.right
                bottomMargin: -margins.bottom
            }
            imagePath: "widgets/lineedit"
            prefix: "hover"
            visible: opacity > 0
            opacity: control.hovered
            Behavior on opacity {
                enabled: control.hovered
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.OutCubic
                }
            }
        }
        PlasmaCore.FrameSvgItem {
            z: hasElement("hint-focus-over-base") ? 0 : -1
            anchors {
                fill: parent
                leftMargin: -margins.left
                topMargin: -margins.top
                rightMargin: -margins.right
                bottomMargin: -margins.bottom
            }
            imagePath: "widgets/lineedit"
            prefix: control.visualFocus && hasElement("focusframe-center") ? "focusframe" : "focus"
            visible: opacity > 0
            opacity: control.visualFocus || control.activeFocus
            Behavior on opacity {
                NumberAnimation {
                    duration: PlasmaCore.Units.longDuration
                    easing.type: Easing.OutCubic
                }
            }
        }
    }

}
