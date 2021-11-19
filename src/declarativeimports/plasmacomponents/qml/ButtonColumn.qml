/*
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1 OR GPL-3.0-only OR LicenseRef-Qt-Commercial
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import "ButtonGroup.js" as Behavior

/**
 * A ButtonColumn allows you to group Buttons in a column.
 *
 * It provides a selection-behavior as well.
 *
 * Note: This component doesn't support the enabled property. If you need to
 * disable it you should disable all the buttons inside it.
 *
 * This is an example:
 *
 * @code
 * ButtonColumn {
 *     Button { text: "Top" }
 *     Button { text: "Bottom" }
 * }
 * @endcode
 *
 * @inherit QtQuick.Column
 */
Column {
    id: root

    /**
     * Specifies the grouping behavior.
     * If enabled, the checked property on buttons contained in the group will
     * be exclusive.
     *
     * The default value is true.
     *
     * Note that a button in an exclusive group will always be checkable.
     */
    property bool exclusive: true

    /**
     * Returns the last checked button.
     */
    property Item checkedButton;

    spacing: PlasmaCore.Theme.defaultFont.pointSize

    Component.onCompleted: {
        Behavior.create(root, {direction: Qt.Vertical});
    }

    Component.onDestruction: {
        Behavior.destroy();
    }

}
