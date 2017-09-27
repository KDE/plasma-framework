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
 * This is a descriptive label which uses the plasma theme. The characteristics of
 * the text will be automatically set according to the plasma theme. Use this
 * components for less important additional data to show in a user interface.
 *
 * Example usage:
 * @code
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 * [...]
 * Column {
 *     PlasmaComponents.Label { text: "Firefox" }
 *     PlasmaExtras.DescriptiveLabel { text: "Web Browser"}
 *   [...]
 * }
 * @endcode
 *
 * See PlasmaComponents Label and primitive QML Text element API for additional
 * properties, methods and signals.
 *
 * @inherits org::kde::plasma::components::Label
 */
Label {
    id: root

    /*
     * If a user can interact with this item, for example in a ListView delegate, this
     * property should be set to true when the label is being interacted with.
     * The default is false
     */
    property bool active: false

    opacity: active ? 0.8 : 0.6
}
