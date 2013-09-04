// -*- coding: iso-8859-1 -*-
/*
 *   Author: Marco Martin <mart@kde.org>
 *   Date: Sun Nov 7 2010, 18:51:24
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    width: 400
    height: 32

    property bool fillWidth: true
    property bool fillHeight: true

    PlasmaCore.DataSource {
        id: tasksSource
        dataEngine: "tasks"
        interval: 0
        onSourceAdded: {
            connectSource(source)
        }
        Component.onCompleted: {
            connectedSources = sources
        }
    }

    function performOperation(src, what) {
        var service = tasksSource.serviceForSource(src);
        var operation = service.operationDescription(what);
        return service.startOperationCall(operation);
    }


    PlasmaExtras.ScrollArea {
        anchors.fill: parent
        ListView {
            //clip: true
            id: tasksList

            orientation: ListView.Horizontal

            model: PlasmaCore.DataModel {
                dataSource: tasksSource
            }

            delegate: MouseArea {
                width: height * 1.6
                height: parent.height

                PlasmaCore.WindowThumbnail {
                    anchors.fill: parent
                    winId: model["DataEngineSource"]
                }

                PlasmaComponents.Label {
                    anchors.bottom: parent.bottom
                    text: visibleName
                }
                PlasmaComponents.Button {
                    text: "Close"
                    anchors.right: parent.right
                }
                onClicked: {
                    performOperation(model["DataEngineSource"], "activate");
                }
            }
        }
    }
}
