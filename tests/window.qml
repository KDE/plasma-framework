
import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.0

import org.kde.plasma.core 2.0 as PlasmaCore

import org.kde.graphicaleffects 1.0

import QtGraphicalEffects 1.15

ApplicationWindow {

    width: 1000
    height: 500
    RowLayout {
        id: topRow
        Label {
            text: "Win ID (find via xwininfo)"
        }
        TextField {
            id: winIdInput
            text: ""
            placeholderText: "0x000000"
        }

        GridLayout {
            columns: 3
            Label {
                text: "Resolution"
            }
            Slider {
                id: resolutionSlider
                from: 0
                to: 1
                value: 0.5
            }
            Label {
                text: resolutionSlider.value
            }
            Label {
                text: "Window Sin C"
            }
            Slider {
                id: windowSincSlider
                from: 0
                to: 1
                value: 0.5
            }
            Label {
                text: windowSincSlider.value
            }
            Label {
                text: "Sin C"
            }
            Slider {
                id: sincSlider
                from: 0
                to: 1
                value: 0.5
            }
            Label {
                text: sincSlider.value
            }
        }
    }



    GridLayout {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.top: topRow.bottom
        height: 200

        columns: 2
        Label {
            text: "Window"
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        Label {
            text: "Lanczos"
            Layout.fillWidth: true
            horizontalAlignment: Text.AlignHCenter
        }

        PlasmaCore.WindowThumbnail {
            id: thumb

            Layout.fillWidth: true
            Layout.fillHeight: true

            winId: parseInt(winIdInput.text, 16)
            visible: true
        }


        Lanczos {
            source: thumb
            Layout.fillWidth: true
            Layout.fillHeight: true
            sourceSize: Qt.size(thumb.paintedWidth, thumb.paintedHeight)
            resolution: resolutionSlider.value
            windowSinc: windowSincSlider.value
            sinc: sincSlider.value
        }
    }
}
