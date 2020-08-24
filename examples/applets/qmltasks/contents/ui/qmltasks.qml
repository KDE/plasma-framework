// -*- coding: iso-8859-1 -*-
/*
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    width: 400
    height: 32

    Layout.fillWidth: true
    Layout.fillHeight: true
    implicitWidth: tasksModel.count * 50

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

    ListView {
        //clip: true
        id: tasksList
        anchors.fill: parent

        orientation: ListView.Horizontal

        model: PlasmaCore.DataModel {
            id: tasksModel
            dataSource: tasksSource
        }

        delegate: PlasmaComponents.ToolButton {
            width: (tasksList.width / (tasksSource.connectedSources.length))
            height: parent.height
            minimumWidth: 24
            minimumHeight: 24
            iconSource: icon
            text: visibleName
            onClicked: {
                performOperation(model["DataEngineSource"], "activate");
            }
        }
    }
}
