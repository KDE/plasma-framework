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

Flipable {
    id: card;
    property string cardPicture: "card.svgz";
    property bool showPicture: false;
    property bool matched: false;
    state: "normal"
    width: cardBack.width
    height: cardBack.height
    
    Behavior on opacity {
        NumberAnimation {
            target: card
            property: "opacity"
            duration: 300
        }
    }
    
    front: Image {
        id: cardBack
        anchors.centerIn: parent
        source: "card.svgz";
    }
    back: Image {
        anchors.centerIn: parent
        source: cardPicture;
    }
    
    transform: Rotation {
         id: rotation
         origin.x: card.width/2
         origin.y: card.height/2
         axis.x: 0; axis.y: 1; axis.z: 0
         angle: 0
         Behavior on angle {
             NumberAnimation {
                 target: rotation
                 property: "angle"
                 duration: 300
            }
        }
     }

     SequentialAnimation {
         id: checkAnimation
         NumberAnimation {
            target: rotation
            property: "angle"
            duration: 300
            from: 0
            to: 180
         }
         PauseAnimation {
            duration: 300
         }
         NumberAnimation {
            target: rotation
            property: "angle"
            duration: 300
            from: 180
            to: 0
         }
     }
    
    MouseArea {
        id: mouseArea;
        anchors.fill: parent;
        onClicked: {
            if (!pairsCanvas.currentCard){
                pairsCanvas.currentCard = card;
                card.state = "checking"
            } else if (pairsCanvas.currentCard != card) {
                if (pairsCanvas.currentCard.cardPicture == card.cardPicture){
                    card.state = pairsCanvas.currentCard.state = "matched"
                    pairsCanvas.matchesCountdown--;
                } else {
                    card.state = pairsCanvas.currentCard.state = "normal"
                    checkAnimation.running = true
                }
                pairsCanvas.currentCard = null;
            }
        }
    }
 
    states: [
        State {
            name: "normal";
            PropertyChanges {
                target: card;
                showPicture: false
            }
            PropertyChanges {
                target: rotation;
                angle: 0
            }
        },
        State {
            name: "checking";
            PropertyChanges {
                target: rotation;
                angle: 180
            }
        },
        State {
            name: "matched";
            PropertyChanges {
                target: card;
                opacity: 0.3
                enabled: false
                showPicture: true
            }
            PropertyChanges {
                target: rotation;
                angle: 180
            }
        }
    ]
}
