/*
    SPDX-FileCopyrightText: 2010 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the QtDeclarative module of the Qt Toolkit.

    SPDX-License-Identifier: LGPL-2.1-only WITH Qt-LGPL-exception-1.1
*/

import Qt 4.7
import Qt.labs.particles 1.0

Item {
    id: block
    property bool dying: false
    property bool spawned: false
    property int type: 0

    Behavior on x {
        enabled: spawned;
        SpringAnimation{ spring: 2; damping: 0.2 }
    }
    Behavior on y {
        SpringAnimation{ spring: 2; damping: 0.2 }
    }

    Image {
        id: img
        source: {
            if(type == 0){
                "pics/redStone.png";
            } else if(type == 1) {
                "pics/blueStone.png";
            } else {
                "pics/greenStone.png";
            }
        }
        opacity: 0
        Behavior on opacity { NumberAnimation { duration: 200 } }
        anchors.fill: parent
    }

    Particles {
        id: particles

        width: 1; height: 1
        anchors.centerIn: parent

        emissionRate: 0
        lifeSpan: 700; lifeSpanDeviation: 600
        angle: 0; angleDeviation: 360;
        velocity: 100; velocityDeviation: 30
        source: {
            if(type == 0){
                "pics/redStar.png";
            } else if (type == 1) {
                "pics/blueStar.png";
            } else {
                "pics/greenStar.png";
            }
        }
    }

    states: [
        State {
            name: "AliveState"; when: spawned == true && dying == false
            PropertyChanges { target: img; opacity: 1 }
        },

        State {
            name: "DeathState"; when: dying == true
            StateChangeScript { script: particles.burst(50); }
            PropertyChanges { target: img; opacity: 0 }
            StateChangeScript { script: block.destroy(1000); }
        }
    ]
}
