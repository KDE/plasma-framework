/*
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

import QtQuick 2.1
import QtQuick.Controls 1.1

import org.kde.plasma.core 2.0 as PlasmaCore

import QtQuick.Controls.Styles.Plasma 2.0 as Styles

/**
 * Simple progressBar using the plasma theme.
 *
 * Some operations take a period of time to be performed and the user needs a
 * confirmation that the operation is still ongoing. If the user does not get
 * any confirmation, they might suspect that they did something wrong or that
 * the device has broken. A progress bar is one of the available mechanisms for
 * providing this reassurance to the user.
 *
 * @inherit QtQuick.Controls.ProgressBar
 */
ProgressBar {
    width: Math.floor(units.gridUnit * (orientation === Qt.Vertical ? 1.6 : 10))
    height: Math.floor(units.gridUnit * (orientation === Qt.Vertical ? 10 : 1.6))
    opacity: enabled ? 1.0 : 0.5

    style: Styles.ProgressBarStyle {}
}
