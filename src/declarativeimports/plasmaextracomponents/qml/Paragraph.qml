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
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.0
import org.kde.plasma.components 2.0

/**
 * This is a paragraph Label which uses the plasma theme.
 *
 * The characteristics of the text will be automatically set according to the
 * plasma theme. Use this components for paragraphs of text in your
 * application. Use this component for blocks of text in your app.
 *
 * Example usage:
 * @code
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 * [...]
 * Column {
 *     PlasmaExtras.Title { text: "Fruit sweetness on the rise" }
 *     PlasmaExtras.Heading { text: "Apples in the sunlight"; level: 2 }
 *     PlasmaExtras.Paragraph { text: "Long text about fruit and apples [...]" }
 *   [...]
 * }
 * @endcode
 *
 * See PlasmaComponents Label and primitive QML Text element API for additional
 * properties, methods and signals.
 */
Label {
    id: paragraph
    width: parent.width
    horizontalAlignment: Text.AlignJustify
    wrapMode: Text.WordWrap
}
