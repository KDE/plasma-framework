/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0

ShaderExample {

    pageName: "Simple"
    pageDescription: "Paints a red, translucent rectangle"

    ShaderEffect {
        anchors.fill: parent
        anchors.topMargin: 48
        opacity: 0.2

        fragmentShader: { " \
            void main(void) { \
                    gl_FragColor = vec4(1.0, 0.0, 0.0, 0.3); \
                } \
            "
        }
    }
}

