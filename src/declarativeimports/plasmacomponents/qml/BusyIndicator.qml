/*
    SPDX-FileCopyrightText: 2010 Artur Duque de Souza <asouzakde.org>
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.2
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

/**
 * A simple busy indicator,
 * It is used to indicate a task whose duration is unknown. If the task
 * duration/number of steps is known, a ProgressBar should be used instead.
 *
 * @inherit QtQuick.Controls.BusyIndicator
 */
BusyIndicator {
    /**
     * Set this property if you don't want to apply a filter to smooth
     * the busy icon while animating.
     * Smooth filtering gives better visual quality, but is slower.
     *
     * The default value is true.
     */
    property bool smoothAnimation: true

    implicitWidth: 52
    implicitHeight: 52

    style: Styles.BusyIndicatorStyle {}

}
