/*   vim:set foldmethod=marker:
 *
 *   Copyright (C) 2013 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2
import org.kde.plasma.shells.desktop 0.1 as Desktop
import org.kde.solidx 0.1 as SolidX

Item {
    id: main

    property bool loaded   : false
    property bool willing  : !solidx.touchscreenPresent
    property int  priority : 1

    signal updated()

    onWillingChanged:  {
        console.log("This is the new status - is desktop shell willing? " + willing)
        main.updated()
    }

    onPriorityChanged: main.updated()

    onLoadedChanged:
        if (loaded) {
            handler.load()
        } else {
            handler.unload()
        }

    Desktop.HandlerObject {
        id: handler
    }

    SolidX.Interface {
        id: solidx
    }
}

