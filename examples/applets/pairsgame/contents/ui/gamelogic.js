/***************************************************************************
 *   Copyright 2010 by Davide Bettio <davide.bettio@kdemail.net>           *
 *   Copyright 2010 by Marco Martin <mart@kde.org>                         *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/


var rows = 3
var columns = 4
var maxVal = rows*columns
var elementsGrid = new Array(rows)
for (var i=0; i < rows; ++i) {
    elementsGrid[i] = new Array(columns)
}

function init()
{
    gameTimer.running = true
    gameTimer.time = 0
    winMessage.y = -winMessage.height
    matchesCountdown = rows*columns / 2;
    var pairs = new Array("bug", "bug", "clock", "clock", "kde", "kde", "konqueror", "konqueror", "magicwand", "magicwand", "plasma", "plasma")
    elements.clear();
    var remaining = maxVal
    for (var i=0; i < maxVal; ++i) {
        var randVal = Math.round((Math.random()*remaining-1))
        
        var element = pairs.splice(randVal, 1)

        --remaining
        elements.append({"card": String(element)})

        elementsGrid[Math.floor(i/columns)][columns%i] = element
    }
}

