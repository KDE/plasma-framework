/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.kirigami 2.20 as Kirigami
import org.kde.ksvg 1.0 as KSvg

Kirigami.Icon {
    property bool selected: false
    property bool isMask: false
    //TODO: implement in libplasma
    property color color: "transparent"
}
