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
     * The level determines how big the section header is display, values
     * between 1 (big) and 5 (small) are accepted. (default: 1)
     */
    property int level: 1

    /**
     * Adjust the point size in between a level and another. (default: 0)
     * @deprecated
     */
    property int step: 0

    enum Type {
        Normal,
        Primary,
        Secondary
    }

    /**
     * The type of the heading. This can be:
     *
     * * PlasmaExtras.Heading.Type.Normal: Create a normal heading (default)
     * * PlasmaExtras.Heading.Type.Primary: Makes the heading more prominent. Useful
     *   when making the heading bigger is not enough.
     * * PlasmaExtras.Heading.Type.Secondary: Makes the heading less prominent.
     *   Useful when an heading is for a less important section in an application.
     *
     * @since 5.88
     */
    property int type: Heading.Type.Normal

    font.pointSize: __headerPointSize(level)
    font.weight: type === Heading.Type.Primary ? Font.DemiBold : Font.Normal
    wrapMode: Text.WordWrap

    opacity: type === Heading.Type.Secondary ? 0.7 : 1

    Accessible.role: Accessible.Heading

    // TODO KF6: Remove this public method
    function headerPointSize(l) {
        console.warn("org.kde.plasma.extras/Heading::headerPointSize() is deprecated. Use font.pointSize directly instead");
        return __headerPointSize(l);
    }

    //
    //  W A R N I N G
    //  -------------
    //
    // This method is not part of the PlasmaExtras API.  It exists purely as an
    // implementation detail.  It may change from version to
    // version without notice, or even be removed.
    //
    // We mean it.
    //
    function __headerPointSize(level) {
        const n = PlasmaCore.Theme.defaultFont.pointSize;
        switch (level) {
        case 1:
            return n * 1.35 + step;
        case 2:
            return n * 1.20 + step;
        case 3:
            return n * 1.15 + step;
        case 4:
            return n * 1.10 + step;
        default:
            return n + step;
        }
    }
}
