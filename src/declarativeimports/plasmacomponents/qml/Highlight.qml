/*
    SPDX-FileCopyrightText: 2022 Aleix Pol Gonzalez <aleixpol@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.plasma.extras 2.0 as PlasmaExtras

/**
 * @see org::kde::plasma::extras::Highlight
 * @deprecated Use org::kde::plasma::extras::Highlight instead.
 */
PlasmaExtras.Highlight
{
    id: root
    // At some point we had the "hover" property which was replaced with "hovered"
    property alias hover: root.hovered
    hovered: true
}
