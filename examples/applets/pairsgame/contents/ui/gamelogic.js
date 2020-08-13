/*
    SPDX-FileCopyrightText: 2010 by Davide Bettio <davide.bettio@kdemail.net>
    SPDX-FileCopyrightText: 2010 by Marco Martin <mart@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/


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

