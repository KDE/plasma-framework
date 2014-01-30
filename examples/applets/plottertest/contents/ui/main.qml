/*
 *   Copyright 2014 Bhushan Shah <bhush94@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.extras 2.0 as PlasmaExtra

PlasmaExtras.SignalPlotter {

	id: plot

	smooth: true

	Timer {
		id: timer
		interval: 500
		running: true
		repeat: true
		onTriggered: {
			plot.addSample(Math.ceil(Math.random()*100));
		}
	}

}