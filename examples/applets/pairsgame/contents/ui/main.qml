/***************************************************************************
 *   Copyright 2010 by Davide Bettio <davide.bettio@kdemail.net>           *
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

import QtQuick 1.0
import "gamelogic.js" as GameLogic


Image {
    id: pairsCanvas;
    width: 800;
    height: 480;
    source: "background.jpg"
    property Item currentCard;
    property int matchesCountdown;

    onMatchesCountdownChanged: {
        if (matchesCountdown == 0) {
            gameTimer.running = false
            winMessage.y = pairsCanvas.height/2 - winMessage.height/2
        }
    }


    Column {
        Image {
            source: "cards-reset.png"
            
            MouseArea {
                anchors.fill: parent;
                onClicked: GameLogic.init();
            } 
        }
        GameTimer {
            id: gameTimer
            width: parent.width
        }
    }
    
    ListModel {
        id: elements
        Component.onCompleted: {
            GameLogic.init()
        }
    }

    Grid {
        id: mainGrid
        anchors.fill: parent
        rows: GameLogic.rows
        columns: GameLogic.columns
        spacing: 5
        Repeater {
            id: repeater
            model: elements
            delegate: Card {
                cardPicture: card+"-card.png"
            }
        }
    }
    
    WinMessage {
        id: winMessage
        anchors.horizontalCenter: mainGrid.horizontalCenter
        y: -height
    }
}
