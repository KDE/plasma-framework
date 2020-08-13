/*
    SPDX-FileCopyrightText: 2013 Digia Plc and /or its subsidiary(-ies) <http://www.qt-project.org/legal>

    This file is part of the examples of the Qt Toolkit.

    SPDX-License-Identifier: BSD-3-Clause
*/

import QtQuick 2.0
import QtQuick.Particles 2.0

ShaderExample {

    pageName: "Particles"
    pageDescription: "Fun rainbow colors using a fragment shader"

    Item {
        anchors.fill: parent
        clip: true
        anchors.margins: -_s
        ParticleSystem {
            id: psItem
            //anchors.fill: parent
            x: parent.width / 2
            y: parent.height / 2
            width: parent.width
            height: parent.height
//             clip: true
            anchors.topMargin: 48
            //anchors.leftMargin: 42

            Emitter {
                emitRate: 400
                lifeSpan: 8000
                size: 24
                sizeVariation: 16
                velocity: PointDirection {x: psItem.width/20; y: psItem.height/20;}
                acceleration: PointDirection {x: -psItem.width/40; y: -psItem.height/40; xVariation: -psItem.width/20; yVariation: -psItem.width/20}
            }

            CustomParticle {
                vertexShader:"
                    uniform lowp float qt_Opacity;
                    varying lowp float fFade;
                    varying highp vec2 fPos;

                    void main() {
                        qt_TexCoord0 = qt_ParticleTex;
                        highp float size = qt_ParticleData.z;
                        highp float endSize = qt_ParticleData.w;

                        highp float t = (qt_Timestamp - qt_ParticleData.x) / qt_ParticleData.y;

                        highp float currentSize = mix(size, endSize, t * t);

                        if (t < 0. || t > 1.)
                        currentSize = 0.;

                        highp vec2 pos = qt_ParticlePos
                        - currentSize / 2. + currentSize * qt_ParticleTex          // adjust size
                        + qt_ParticleVec.xy * t * qt_ParticleData.y         // apply velocity vector..
                        + 0.5 * qt_ParticleVec.zw * pow(t * qt_ParticleData.y, 2.);

                        gl_Position = qt_Matrix * vec4(pos.x, pos.y, 0, 1);

                        highp float fadeIn = min(t * 20., 1.);
                        highp float fadeOut = 1. - max(0., min((t - 0.75) * 4., 1.));

                        fFade = fadeIn * fadeOut * qt_Opacity;
                        fPos = vec2(pos.x/320., pos.y/480.);
                    }
                "
                //! [0]
                fragmentShader: "
                    varying highp vec2 fPos;
                    varying lowp float fFade;
                    varying highp vec2 qt_TexCoord0;
                    void main() {//*2 because this generates dark colors mostly
                        highp vec2 circlePos = qt_TexCoord0*2.0 - vec2(1.0,1.0);
                        highp float dist = length(circlePos);
                        highp float circleFactor = max(min(1.0 - dist, 1.0), 0.0);
                        gl_FragColor = vec4(fPos.x*2.0 - fPos.y, fPos.y*2.0 - fPos.x, fPos.x*fPos.y*2.0, 0.0) * circleFactor * fFade;
                    }"
                //! [0]

            }
        }
    }
}
