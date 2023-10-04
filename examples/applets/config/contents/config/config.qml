/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick
import org.kde.plasma.configuration

ConfigModel {
    ConfigCategory {
         name: "General"
         icon: "plasma"
         source: "configGeneral.qml"
    }
    ConfigCategory {
         name: "Other page"
         icon: "konqueror"
         source: "configOther.qml"
    }
}
