/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore

T.Label {
    id: control

    // Work around Qt bug where left aligned text is not right aligned
    // in RTL mode unless horizontalAlignment is explicitly set.
    // https://bugreports.qt.io/browse/QTBUG-95873
    horizontalAlignment: Text.AlignLeft
    verticalAlignment: lineCount > 1 ? Text.AlignTop : Text.AlignVCenter

    activeFocusOnTab: false

    //font data is the system one by default
    color: PlasmaCore.ColorScope.textColor
    linkColor: PlasmaCore.Theme.linkColor

    opacity: enabled ? 1 : 0.6

    Accessible.role: Accessible.StaticText
    Accessible.name: text
}
