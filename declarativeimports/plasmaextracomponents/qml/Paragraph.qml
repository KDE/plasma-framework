/*
*   Copyright 2012 by Sebastian Kügler <sebas@kde.org>
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
        Label

Imports:
        QtQuick 1.1
        org.kde.plasma.extras

Description:
    This is a paragraph Label which uses the plasma theme. The characteristics of the text will
    be automatically set according to the plasma theme. Use this components for paragraphs
    of text in your application. Use this component for blocks of text in your app.

    Example usage:
    <code>
import org.kde.plasma.extras 0.1 as PlasmaExtras
[...]
Column{
    PlasmaExtras.Title { text: "Fruit sweetness on the rise" }
    PlasmaExtras.Heading { text: "Apples in the sunlight"; level: 2 }
    PlasmaExtras.Paragraph { text: "Long text about fruit and apples [...]" }
  [...]
}
</code>

    See Plasma Component's Label and primitive QML Text element API for additional properties,
    methods and signals.

Properties:
    string text:
    The most important property is "text", which applies to the text property of Label
    For the other ones see Plasma Component's Label or QML primitive Text element

    int level:
    The level determines how big the section header is display, values between 1 (big)
    and 5 (small) are accepted

Methods:

Signals:
    See Plasma Component's Label and primitive QML Text element
**/

import QtQuick 1.1
import org.kde.plasma.components 0.1

Label {
    id: paragraph
    width: parent.width
    horizontalAlignment: Text.AlignJustify
    wrapMode: Text.WordWrap
}
