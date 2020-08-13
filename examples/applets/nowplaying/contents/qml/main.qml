/*
    SPDX-FileCopyrightText: 2010 Marco Martin <notmart@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts


QGraphicsWidget {
    id: page;
    preferredSize: "200x200"
    minimumSize: "200x20"
    property string activeSource: ''

    Item {
        id:main

        PlasmaCore.DataSource {
            id: dataSource
            dataEngine: "nowplaying"
            connectedSources: ['players']
            interval: 500

            onNewData: {
                if (sourceName == 'players') {
                    print("going to connect to " +  data['players'][0])
                    activeSource = data['players'][0]
                    connectedSources = ['players', activeSource]
                }
            }

            onDataChanged: {
                if (!activeSource) {
                    return;
                }

                if (data[activeSource].State == "playing") {
                    playPause.setIcon("media-playback-pause")
                } else {
                    playPause.setIcon("media-playback-start")
                }

                progress.value = 100*data[activeSource].Position/data[activeSource].Length
            }
        }

        PlasmaCore.Theme {
                id: theme
        }
    }

    Component.onCompleted:
    {
        dataSource.serviceForSource(activeSource).associateWidget(stop, "stop");
        dataSource.serviceForSource(activeSource).associateWidget(progress, "progress");
    }

    layout: GraphicsLayouts.QGraphicsLinearLayout {

        PlasmaWidgets.IconWidget {
            id: playPause
            property string state: "stop"

            onClicked: {
                var operation;
                if (dataSource.data[activeSource].State == "playing") {
                    operation = "pause"
                } else {
                    operation = "play"
                }
                var data = dataSource.serviceForSource(activeSource).operationDescription(operation);
                print(dataSource.serviceForSource(activeSource).name);

                for ( var i in data ) {
                    print(i + ' -> ' + data[i] );
                }

                dataSource.serviceForSource(activeSource).startOperationCall(dataSource.serviceForSource(activeSource).operationDescription(operation));
                print("stopping");
            }
        }

        PlasmaWidgets.IconWidget {
            id: stop
            Component.onCompleted: {
                setIcon("media-playback-stop");
            }
            onClicked: {
                var data = dataSource.serviceForSource(activeSource).operationDescription("stop");
                print(dataSource.serviceForSource(activeSource).name);

                for ( var i in data ) {
                    print(i + ' -> ' + data[i] );
                }

                dataSource.serviceForSource(activeSource).startOperationCall(dataSource.serviceForSource(activeSource).operationDescription("stop"));
                print("stopping");
            }
        }

        PlasmaWidgets.Slider {
            id: progress
            orientation: Qt.Horizontal

            onSliderMoved: {
                var operation = dataSource.serviceForSource(activeSource).operationDescription("seek");
                operation.seconds = Math.round(dataSource.data[activeSource].Length*(value/100));

                for ( var i in operation ) {
                    print(i + ' -> ' + operation[i] );
                }

                dataSource.serviceForSource(activeSource).startOperationCall(operation);
                print("set progress to " + progress);
            }
        }
    }
}
