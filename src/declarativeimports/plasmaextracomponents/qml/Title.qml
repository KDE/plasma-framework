/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.components 2.0

/**
 * This is a title label which uses the plasma theme.  The characteristics of
 * the text will be automatically set according to the plasma theme. Use this
 * components for titles in your UI, for example page or section titles.
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
 * @deprecated use org.kde.Kirigami.Heading instead
 */
Heading {
}
