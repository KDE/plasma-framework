/*
    SPDX-FileCopyrightText: 2010 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
