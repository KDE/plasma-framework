/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2022 Carl Schwan <carl@carlschwan.eu>
    SPDX-FileCopyrightText: 2023 ivan tkachenko <me@ratijas.tk>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import org.kde.ksvg 1.0 as KSvg
import org.kde.kirigami 2 as Kirigami

/**
 * @brief Highlight for a list or grid item.
 *
 * Highlight provides the highlight used to indicate the active
 * item in a model view. It is typically used in conjunction with
 * the @sa QtQuick.ListView::highlight or the
 * @sa QtQuick.GridView::highlight properties.
 *
 * Provides built-in animation of Behavior on opacity Easing.OutQuad for a
 * duration of 50ms (defined in Kirigami.Units.veryShortDuration).
 *
 * @code{.qml}
 * import QtQuick 2.15
 * import org.kde.plasma.extras 2.0 as PlasmaExtras
 *
 * ListView {
 *     highlightFollowsCurrentItem: true
 *     highlight: PlasmaExtras.Highlight { }
 *     highlightMoveDuration: 0
 *     highlightResizeDuration: 0
 *     currentIndex: -1
 * }
 *
 * @endcode
 *
 * @inherit QtQuick.Item
 */
Item {
    id: highlight

    /**
     * This property holds whether the control is hovered.
     *
     * This is set automatically when used in a ListView and GridView.
     */
    property bool hovered: ListView.view !== null || GridView.view !== null

    /**
     * This property holds whether the highlight has a pressed appearance.
     */
    property bool pressed: false

    /**
	 * This property holds the margin hints used by the background.
     *
	 * @property int marginHints
	 */
    property alias marginHints: background.margins

    /**
     * This property holds whether the item is active. True by default. Set it to
     * false to visually mark an item that's in the "current item" or "selected"
     * state but is not currently being hovered.
     */
    property bool active: true

    width: ListView.view?.width ?? undefined

    KSvg.FrameSvgItem {
        id: background

        anchors.fill: parent

        opacity: highlight.active ? 1 : 0.6

        imagePath: "widgets/viewitem"
        prefix: {
            if (highlight.pressed) {
                return highlight.hovered ? 'selected+hover' : 'selected';
            }

            return highlight.hovered ? 'hover' : 'normal';
        }

        Behavior on opacity {
            NumberAnimation {
                duration: Kirigami.Units.veryShortDuration
                easing.type: Easing.OutQuad
            }
        }
    }
}
