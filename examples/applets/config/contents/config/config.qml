/*
    SPDX-FileCopyrightText: 2013 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

import org.kde.plasma.configuration 2.0

ConfigModel {
    ConfigCategory {
         name: "General"
         icon: "plasma"
         source: "configGeneral.qml"
    }
    ConfigCategory {
         name: "Other page"
         icon: "konqueror"
         source: "configSecondPage.qml"
    }
}
