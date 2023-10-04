/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick
import org.kde.kirigami as Kirigami
import org.kde.plasma.components as PlasmaComponents

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
 * import org.kde.plasma.components as PlasmaComponents
 * import org.kde.plasma.extras as PlasmaExtras
 *
 * Column {
 *     PlasmaExtras.Heading { text: "Fruit sweetness on the rise"; level: 1 }
 *     PlasmaExtras.Heading { text: "Apples in the sunlight"; level: 2 }
 *     PlasmaComponents.Label { text: "Long text about fruit and apples [...]" }
 * }
 * @endcode
 *
 * The most important property is "text", which applies to the text property of
 * Label. See PlasmaComponents Label and primitive QML Text element API for
 * additional properties, methods and signals.
 */
PlasmaComponents.Label {
    /**
     * @brief This property holds the level of the heading, which determines its size.
     *
     * This property holds the level, which determines how large the header is.
     *
     * Acceptable values range from 1 (big) to 5 (small).
     *
     * default: ``1``
     */
    property int level: 1

    /**
     * @brief This enumeration defines heading types.
     *
     * This enum helps with heading visibility (making it less or more important).
     */
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

    font.pointSize: {
        let factor = 1;
        switch (level) {
            case 1:
                factor = 1.35;
                break;
            case 2:
                factor = 1.20;
                break;
            case 3:
                factor = 1.15;
                break;
            case 4:
                factor = 1.10;
                break;
            default:
                break;
        }
        return Kirigami.Theme.defaultFont.pointSize * factor;
    }
    font.weight: type === Heading.Type.Primary ? Font.DemiBold : Font.Normal

    opacity: type === Heading.Type.Secondary ? 0.7 : 1

    Accessible.role: Accessible.Heading
}
