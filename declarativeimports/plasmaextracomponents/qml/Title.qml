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

/**Documented API
Inherits:
        Text

Imports:
        QtQuick 1.1
        org.kde.plasma.core

Description:
    This is a label which uses the plasma theme.
    The characteristics of the text will be automatically seted
    according to the plasma theme. If you need a more customized
    text,then you can use the Text component from QtQuick.

Properties:
    string text:
    The most important property is "text".
    For the other ones see the primitive QML Text element

Methods:
    See the primitive QML Text element

Signals:
    See the primitive QML Text element
**/

import QtQuick 1.1
import org.kde.plasma.components 0.1

Label {
    id: root

    font.pointSize: theme.defaultFont.pointSize*2
}
