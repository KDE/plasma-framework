/*
*   Copyright (C) 2011 by Marco Martin <mart@kde.org>
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

/**Documentanted API
Inherits:
        Item

Imports:
        QtQuick 1.0
        org.kde.plasma.core

Description:
 TODO i need more info here
**/

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore

Text {
    id: root

    font.capitalization: theme.defaultFont.capitalization
    font.family: theme.defaultFont.family
    font.italic: theme.defaultFont.italic
    font.letterSpacing: theme.defaultFont.letterSpacing
    font.pointSize: theme.defaultFont.pointSize
    font.strikeout: theme.defaultFont.strikeout
    font.underline: theme.defaultFont.underline
    font.weight: theme.defaultFont.weight
    font.wordSpacing: theme.defaultFont.wordSpacing
    color: theme.textColor

    wrapMode: Text.Wrap
}
