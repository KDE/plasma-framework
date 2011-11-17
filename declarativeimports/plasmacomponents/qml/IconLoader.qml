/*
*   Copyright (C) 2011 by Marco MArtin <mart@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
*   (at your option) any later version.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU Library General Public License for more details
*
*   You should have received a copy of the GNU Library General Public
*   License along with this program; if not, write to the
*   Free Software Foundation, Inc.,
*   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

Item {
    id: root

    property bool valid: false

    property string source

    onSourceChanged: {
        if (source == "") {
            imageLoader.sourceComponent = null
            valid = false
        }

        svgIcon.imagePath = "toolbar-icons/"+root.source.split("-")[0]
        if (!svgIcon.isValid() || !svgIcon.hasElement(root.source)) {
            svgIcon.imagePath = "icons/"+root.source.split("-")[0]
        }

        if (svgIcon.isValid() && svgIcon.hasElement(root.source)) {
            imageLoader.sourceComponent = svgComponent
        } else if (root.source.indexOf(".") == -1 && root.source.indexOf(":") == -1) {
            imageLoader.sourceComponent = iconComponent
        } else {
            imageLoader.sourceComponent = imageComponent
        }
        valid = true
    }

    implicitWidth: theme.smallIconSize
    implicitHeight: theme.smallIconSize

    PlasmaCore.Svg {
        id: svgIcon
    }

    Loader {
        id: imageLoader
        anchors.fill: parent

        Component {
            id: svgComponent

            PlasmaCore.SvgItem {
                svg: svgIcon
                elementId: root.source
                anchors.fill: parent
            }
        }
        
        Component {
            id: iconComponent

            QIconItem {
                icon: QIcon(root.source)
                smooth: true
                anchors.fill: parent
            }
        }

        Component {
            id: imageComponent

            Image {
                source: root.source
                sourceSize.width: width
                sourceSize.height: height
                fillMode: Image.PreserveAspectFit
                smooth: true
                anchors.fill: parent
            }
        }
    }
}
