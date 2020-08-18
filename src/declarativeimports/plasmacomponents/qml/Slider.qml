/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private
import QtQuick.Controls 1.2 as QtControls
import QtQuick.Controls.Styles.Plasma 2.0 as Styles


/**
 * An interactive slider component with Plasma look and feel.
 *
 * @inherit QtQuick.Controls.Slider
 */
QtControls.Slider {
    id: slider

    /**
     * This property holds if a value indicator element will be shown while is
     * dragged or not.
     *
     * @warning The value indicator is not implemented in the Plasma Slider.
     *
     * The default value is false.
     */
    property bool valueIndicatorVisible: false

    /**
     * This property holds the text being displayed in the value indicator.
     *
     * @warning The value indicator is not implemented in the Plasma Slider.
     */
    property string valueIndicatorText: value

    /**
     * type:bool
     * This property holds if the slider visualizations has an inverted
     * direction.
     *
     * @warning: deprecated and not supported, here for retrocompatibility
     */
    property bool inverted: false

    implicitWidth: slider.isVertical ? PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height*1.6 : 200
    implicitHeight: slider.isVertical ? 200 : PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height*1.6
    // TODO: needs to define if there will be specific graphics for
    //     disabled sliders
    opacity: enabled ? 1.0 : 0.5

    activeFocusOnTab: true

    //FIXME: remove those 2 functions once we can depend from 5.4*/
    function accessibleIncreaseAction() { increase() }
    function accessibleDecreaseAction() { decrease() }

    function increase() {
        if (!enabled)
            return;
        if (inverted)
            value += stepSize;
        else
            value -= stepSize;
    }
    function decrease() {
        if (!enabled)
            return;
        if (inverted)
            value -= stepSize;
        else
            value += stepSize;
    }

    style: Styles.SliderStyle {}
}
