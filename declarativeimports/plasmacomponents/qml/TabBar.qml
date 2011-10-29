/****************************************************************************
**
** Copyright 2011 Marco Martin <mart@kde.org>
**
** Copyright (C) 2011 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the Qt Components project.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: root
    default property alias content: tabBarLayout.data
    property alias layout: tabBarLayout

    //Plasma extension
    property Item currentItem

    implicitWidth: tabBarLayout.implicitWidth + backgroundFrame.margins.left + backgroundFrame.margins.right + buttonFrame.margins.left + buttonFrame.margins.right
    implicitHeight: tabBarLayout.implicitHeight + backgroundFrame.margins.top + backgroundFrame.margins.bottom + buttonFrame.margins.top + buttonFrame.margins.bottom

    PlasmaCore.FrameSvgItem {
        id: backgroundFrame

        anchors.fill: parent
        imagePath: "widgets/frame"
        prefix: "sunken"
    }

    PlasmaCore.FrameSvgItem {
        id: buttonFrame

        x: currentItem.x + backgroundFrame.margins.left
        y: backgroundFrame.margins.top
        width: currentItem.width + buttonFrame.margins.left + buttonFrame.margins.right
        height: currentItem.height + buttonFrame.margins.top + buttonFrame.margins.bottom
        imagePath: "widgets/button"
        prefix: "normal"
        Behavior on x {
            PropertyAnimation {
                easing.type: Easing.InQuad
                duration: 250
            }
        }
    }

    TabBarLayout {
        id: tabBarLayout
        anchors {
            fill: parent
            leftMargin: backgroundFrame.margins.left + buttonFrame.margins.left
            topMargin: backgroundFrame.margins.top + buttonFrame.margins.top
            rightMargin: backgroundFrame.margins.right + buttonFrame.margins.right
            bottomMargin: backgroundFrame.margins.bottom + buttonFrame.margins.bottom
        }
    }
}
