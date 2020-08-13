/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>
    SPDX-FileCopyrightText: 2014 Kai Uwe Broulik <kde@privat.broulik.de>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
    width: Math.floor(PlasmaCore.Units.gridUnit * (orientation === Qt.Vertical ? 1.6 : 10))
    height: Math.floor(PlasmaCore.Units.gridUnit * (orientation === Qt.Vertical ? 10 : 1.6))
    opacity: enabled ? 1.0 : 0.5

    style: Styles.ProgressBarStyle {}
}
