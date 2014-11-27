/*
*   Copyright (C) 2010 by Artur Duque de Souza <asouzakde.org>
*   Copyright (C) 2011 by Daker Fernandes Pinheiro <dakerfp@gmail.com>
*   Copyright (C) 2014 by Kai Uwe Broulik <kde@privat.broulik.de>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
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
