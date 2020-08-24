/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Window 2.2
import QtQuick.Controls 1.0
import QtQuick.Controls.Private 1.0 as QtQuickControlsPrivate
import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * This is a label which uses the plasma theme.
 *
 * The characteristics of the text will be automatically set according to the
 * plasma theme. If you need a more customized text item use the Text component
 * from QtQuick.
 *
 * You can use all elements of the QML Text component, in particular the "text"
 * property to define the label text.
 *
 * @inherit QtQuick.Text
 */
Text {
    id: root

    height: Math.round(Math.max(paintedHeight, PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height*1.6))
    verticalAlignment: lineCount > 1 ? Text.AlignTop : Text.AlignVCenter

    activeFocusOnTab: false

    // Work around Qt bug where NativeRendering breaks for non-integer scale factors
    // https://bugreports.qt.io/browse/QTBUG-70481
    renderType: QtQuickControlsPrivate.Settings.isMobile || Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering

    font.capitalization: PlasmaCore.Theme.defaultFont.capitalization
    font.family: PlasmaCore.Theme.defaultFont.family
    font.italic: PlasmaCore.Theme.defaultFont.italic
    font.letterSpacing: PlasmaCore.Theme.defaultFont.letterSpacing
    font.pointSize: PlasmaCore.Theme.defaultFont.pointSize
    font.strikeout: PlasmaCore.Theme.defaultFont.strikeout
    font.underline: PlasmaCore.Theme.defaultFont.underline
    font.weight: PlasmaCore.Theme.defaultFont.weight
    font.wordSpacing: PlasmaCore.Theme.defaultFont.wordSpacing
    color: PlasmaCore.ColorScope.textColor

    opacity: enabled? 1 : 0.6

    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
