// -*- coding: iso-8859-1 -*-
/*
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import QtQuick.Layouts 1.1
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras

Item {
    width: 400
    height: 32

    Layout.fillWidth: true
    Layout.fillHeight: true

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

    function performOperation(id, what) {
        var service = tasksSource.serviceForSource("tasks");
        var operation = service.operationDescription(what);
        operation["Id"] = id
        return service.startOperationCall(operation);
    }


    PlasmaExtras.ScrollArea {
        anchors.fill: parent
        ListView {
            //clip: true
            id: tasksList

            orientation: ListView.Horizontal

            model: tasksSource.models["tasks"]

            delegate: MouseArea {
                width: height * 1.6
                height: parent.height

                PlasmaCore.WindowThumbnail {
                    anchors.fill: parent
                    winId: model["WindowList"][0]
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
                    performOperation(model["Id"], "activate");
                }
            }
        }
    }
}
