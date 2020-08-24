/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0


/**
 * A heading label used for subsections of texts.
 *
 * The characteristics of the text will be automatically set according to the
 * plasma theme. Use this components for section titles or headings in your UI,
 * for example page or section titles.
 *
 * Example usage:
 *
 * @code
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 * [...]
 * Column {
 *     PlasmaExtras.Heading { text: "Fruit sweetness on the rise"; level: 1 }
 *     PlasmaExtras.Heading { text: "Apples in the sunlight"; level: 2 }
 *     PlasmaExtras.Paragraph { text: "Long text about fruit and apples [...]" }
 *   [...]
 * }
 * @endcode
 *
 * The most important property is "text", which applies to the text property of
 * Label. See PlasmaComponents Label and primitive QML Text element API for
 * additional properties, methods and signals.
 */
Label {
    id: heading

    /**
     * level: int
     * The level determines how big the section header is display, values
     * between 1 (big) and 5 (small) are accepted
     */
    property int level: 1

    /**
     * step: int
     * adjust the point size in between a level and another.
     */
    property int step: 0

    lineHeight: 1.2
    font.pointSize: headerPointSize(level)
    wrapMode: Text.WordWrap

    function headerPointSize(l) {
        var n = PlasmaCore.Theme.defaultFont.pointSize;
        var s;
        switch (l) {
        case 1:
            return Math.round(n * 1.50) + step;
        case 2:
            return Math.round(n * 1.30) + step;
        case 3:
            return Math.round(n * 1.20) + step;
        case 4:
            return Math.round(n * 1.10) + step;
        default:
            return n + step;
        }
    }
}
