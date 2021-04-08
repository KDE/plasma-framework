/*
    SPDX-FileCopyrightText: 2015 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

pragma Singleton

import QtQuick 2.4
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.4

/**
 * A set of values to define semantically sizes and durations
 * @inherit QtQuick.QtObject
 */
QtObject {
    /**
     * The fundamental unit of space that should be used for sizes, expressed in pixels.
     * Given the screen has an accurate DPI settings, it corresponds to a width of
     * the capital letter M
     */
    property int gridUnit: PlasmaCore.Units.gridUnit

    /**
     * PlasmaCore.Units.iconSizes provides access to platform-dependent icon sizing
     *
     * The icon sizes provided are normalized for different DPI, so icons
     * will scale depending on the DPI.
     *
     * Icon sizes from KIconLoader, adjusted to devicePixelRatio:
     * * small
     * * smallMedium
     * * medium
     * * large
     * * huge
     * * enormous
     *
     * Not devicePixelRation-adjusted::
     * * desktop
     */
    property QtObject iconSizes: QtObject {
        property int sizeForLabels: fontMetrics.roundedIconSize(fontMetrics.height)
        property int small: Math.floor(fontMetrics.roundedIconSize(16 * devicePixelRatio) * (Settings.isMobile ? 1.5 : 1))
        property int smallMedium: Math.floor(fontMetrics.roundedIconSize(22 * devicePixelRatio) * (Settings.isMobile ? 1.5 : 1))
        property int medium: Math.floor(fontMetrics.roundedIconSize(32 * devicePixelRatio) * (Settings.isMobile ? 1.5 : 1))
        property int large: Math.floor(fontMetrics.roundedIconSize(48 * devicePixelRatio) * (Settings.isMobile ? 1.5 : 1))
        property int huge: Math.floor(fontMetrics.roundedIconSize(64 * devicePixelRatio) * (Settings.isMobile ? 1.5 : 1))
        property int enormous: Math.floor(128 * devicePixelRatio * (Settings.isMobile ? 1.5 : 1))
    }

    /**
     * PlasmaCore.Units.smallSpacing is the amount of spacing that should be used around smaller UI elements,
     * for example as spacing in Columns. Internally, this size depends on the size of
     * the default font as rendered on the screen, so it takes user-configured font size and DPI
     * into account.
     */
    property int smallSpacing: Math.floor(gridUnit/4)

    /**
     * PlasmaCore.Units.largeSpacing is the amount of spacing that should be used inside bigger UI elements,
     * for example between an icon and the corresponding text. Internally, this size depends on
     * the size of the default font as rendered on the screen, so it takes user-configured font
     * size and DPI into account.
     */
    property int largeSpacing: smallSpacing * 2

    /**
     * The ratio between physical and device-independent pixels. This value does not depend on the \
     * size of the configured font. If you want to take font sizes into account when scaling elements,
     * use PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont), PlasmaCore.Units.smallSpacing and PlasmaCore.Units.largeSpacing.
     * The devicePixelRatio follows the definition of "device independent pixel" by Microsoft.
     */
    property real devicePixelRatio: PlasmaCore.Units.devicePixelRatio

    /**
     * PlasmaCore.Units.longDuration should be used for longer, screen-covering animations, for opening and
     * closing of dialogs and other "not too small" animations
     */
    property int longDuration: PlasmaCore.Units.longDuration

    /**
     * PlasmaCore.Units.shortDuration should be used for short animations, such as accentuating a UI event,
     * hover events, etc..
     */
    property int shortDuration: PlasmaCore.Units.shortDuration

    /**
     * PlasmaCore.Units.veryShortDuration should be used for elements that should have a hint of smoothness,
     * but otherwise animate near instantly.
     */
    property int veryShortDuration: PlasmaCore.Units.veryShortDuration

    /**
     * PlasmaCore.Units.veryLongDuration should be used for specialty animations that benefit
     * from being even longer than longDuration.
     */
    property int veryLongDuration: PlasmaCore.Units.veryLongDuration

    /**
     * How much the mouse scroll wheel scrolls, expressed in lines of text.
     * Note: this is strictly for classical mouse wheels, touchpads 2 figer scrolling won't be affected
     */
    property int wheelScrollLines: 3

    /**
     * time in ms by which the display of tooltips will be delayed.
     *
     * @sa ToolTip.delay property
     */
    property int toolTipDelay: Settings.tabletMode ? Qt.styleHints.mousePressAndHoldInterval : 700

    /**
     * Time in milliseconds equivalent to the theoretical human moment, which can be used
     * to determine whether how long to wait until the user should be informed of something,
     * or can be used as the limit for how long something should wait before being
     * automatically initiated.
     *
     * Some examples:
     *
     * - When the user types text in a search field, wait no longer than this duration after
     *   the user completes typing before starting the search
     * - When loading data which would commonly arrive rapidly enough to not require interaction,
     *   wait this long before showing a spinner
     *
     * This might seem an arbitrary number, but given the psychological effect that three
     * seconds seems to be what humans consider a moment (and in the case of waiting for
     * something to happen, a moment is that time when you think "this is taking a bit long,
     * isn't it?"), the idea is to postpone for just before such a conceptual moment. The reason
     * for the two seconds, rather than three, is to function as a middle ground: Not long enough
     * that the user would think that something has taken too long, for also not so fast as to
     * happen too soon.
     *
     * See also
     * https://www.psychologytoday.com/blog/all-about-addiction/201101/tick-tock-tick-hugs-and-life-in-3-second-intervals
     * (the actual paper is hidden behind an academic paywall and consequently not readily
     * available to us, so the source will have to be the blog entry above)
     *
     * @since 5.81
     */
    property int humanMoment: 2000

    /**
     * metrics used by the default font
     */
    property variant fontMetrics: TextMetrics {
        text: "M"
        function roundedIconSize(size) {
            if (size < 16) {
                return size;
            } else if (size < 22) {
                return 16;
            } else if (size < 32) {
                return 22;
            } else if (size < 48) {
                return 32;
            } else if (size < 64) {
                return 48;
            } else {
                return size;
            }
        }
    }
}
