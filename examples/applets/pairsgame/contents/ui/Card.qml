/*
    SPDX-FileCopyrightText: 2010 Davide Bettio <davide.bettio@kdemail.net>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

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
