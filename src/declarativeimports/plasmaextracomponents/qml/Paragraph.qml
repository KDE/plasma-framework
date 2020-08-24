/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
 *     PlasmaExtras.Heading { text: "Fruit sweetness on the rise"; level: 1 }
 *     PlasmaExtras.Heading { text: "Apples in the sunlight"; level: 2 }
 *     PlasmaExtras.Paragraph { text: "Long text about fruit and apples [...]" }
 *   [...]
 * }
 * @endcode
 *
 * See PlasmaComponents Label and primitive QML Text element API for additional
 * properties, methods and signals.
 * @deprecated use QtQuickControls 2.0 Label instead
 */
Label {
    id: paragraph
    width: parent.width
    horizontalAlignment: Text.AlignJustify
    wrapMode: Text.WordWrap
}
