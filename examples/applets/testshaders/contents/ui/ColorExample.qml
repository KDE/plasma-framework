/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

ShaderExample {

    pageName: "Colors"
    pageDescription: ""

    ShaderEffect {
        anchors.fill: parent
        anchors.topMargin: 48
        opacity: 0.2

        fragmentShader: {
            "uniform mat4 gl_ModelViewMatrix;" +
            "uniform mat4 gl_ProjectionMatrix;" +
            //"attribute vec4 gl_Vertex;" +
            "void main(void) {" +
            "   gl_FragColor = vec4(1.0, 0.0, 0.0, 0.3); " +
            "   gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; " +
            "}"
        }
    }
}

