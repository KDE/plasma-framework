/*
    SPDX-FileCopyrightText: 2013 Digia Plc and /or its subsidiary(-ies) <http://www.qt-project.org/legal>

    This file is part of the examples of the Qt Toolkit.

    SPDX-License-Identifier: BSD-3-Clause
*/

import QtQuick 2.0
import QtQuick.Particles 2.0

Item {
    width: 360
    height: 600

    Image {
        source: "../images/backgroundLeaves.jpg"
        anchors.fill: parent
    }
    ParticleSystem {
        anchors.fill: parent
        Emitter {
            width: parent.width
            emitRate: 4
            lifeSpan: 14000
            size: 80
            velocity: PointDirection { y: wallpaper.configuration.Speed }
        }
        Wander {
            anchors.fill: parent
            anchors.bottomMargin: 100
            xVariance: 60
            pace: 60
        }

        //! [0]
        Affector {
            property real coefficient: 2.0
            property real velocity: 1.5
            width: parent.width
            height: parent.height - 100
            onAffectParticles: {
            /*  //Linear movement
                if (particle.r == 0) {
                    particle.r = Math.random() > 0.5 ? -1 : 1;
                } else if (particle.r == 1) {
                    particle.rotation += velocity * dt;
                    if (particle.rotation >= maxAngle)
                        particle.r = -1;
                } else if (particle.r == -1) {
                    particle.rotation -= velocity * dt;
                    if (particle.rotation <= -1 * maxAngle)
                        particle.r = 1;
                }
            */
                //Wobbly movement
                for (var i=0; i<particles.length; i++) {
                    var particle = particles[i];
                    if (particle.r == 0.0) {
                        particle.r = Math.random() + 0.01;
                    }
                    particle.rotation += velocity * particle.r * dt;
                    particle.r -= particle.rotation * coefficient;
                    if (particle.r == 0.0)
                        particle.r -= particle.rotation * 0.000001;
                    particle.update = 1;
                }
            }
        }
        //! [0]

        //! [1]
        Affector {//Custom Friction, adds some 'randomness'
            x: -60
            width: parent.width + 120
            height: 100
            anchors.bottom: parent.bottom
            onAffectParticles: {
                for (var i=0; i<particles.length; i++) {
                    var particle = particles[i];
                    var pseudoRand = (Math.floor(particle.t*1327) % 10) + 1;
                    var yslow = dt * pseudoRand * 0.5 + 1;
                    var xslow = dt * pseudoRand * 0.05 + 1;
                    if (particle.vy < 1)
                        particle.vy = 0;
                    else
                        particle.vy = (particle.vy / yslow);
                    if (particle.vx < 1)
                        particle.vx = 0;
                    else
                        particle.vx = (particle.vx / xslow);
                    particle.update = true;
                }
            }
        }
        //! [1]

        ImageParticle {
            anchors.fill: parent
            id: particles
            sprites: [Sprite {
                    source: "../images/realLeaf1.png"
                    frameCount: 1
                    frameDuration: 1
                    to: {"a":1, "b":1, "c":1, "d":1}
                }, Sprite {
                    name: "a"
                    source: "../images/realLeaf1.png"
                    frameCount: 1
                    frameDuration: 10000
                },
                Sprite {
                    name: "b"
                    source: "../images/realLeaf2.png"
                    frameCount: 1
                    frameDuration: 10000
                },
                Sprite {
                    name: "c"
                    source: "../images/realLeaf3.png"
                    frameCount: 1
                    frameDuration: 10000
                },
                Sprite {
                    name: "d"
                    source: "../images/realLeaf4.png"
                    frameCount: 1
                    frameDuration: 10000
                }
            ]

            z:4
        }
    }
}
