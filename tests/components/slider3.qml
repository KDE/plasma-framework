import QtQuick 2.0
import QtQuick.Layouts 1.3

import org.kde.kirigami 2.5 as Kirigami
import org.kde.plasma.components 3.0
import QtQuick.Controls 2.5 as QQC2
import org.kde.plasma.components 2.0 as PC2

// Run with qmlscene to use qqc2-desktop-style

Kirigami.ApplicationWindow {
    pageStack.initialPage: Kirigami.Page {
        Kirigami.FormLayout {
            anchors.fill: parent
            PC2.Slider {
                Layout.fillWidth: true
                Kirigami.FormData.label: "PC2 slider"
                maximumValue: slider.to
                stepSize: slider.stepSize
            }
            QQC2.Slider {
                Layout.fillWidth: true
                Kirigami.FormData.label: "QQC2 slider"
                to: slider.to
                stepSize: slider.stepSize
            }
            Slider {
                id: slider
                Kirigami.FormData.label: "PC3 slider"
                to: max.text
                stepSize: 1
                clip: true
            }
            TextField {
                id: max
                Kirigami.FormData.label: "maximumValue: "
                text: "100"
            }
            Label {
                Kirigami.FormData.label: "value: "
                text: slider.value
            }
            Slider {
                Kirigami.FormData.label: "Choose step size: "
                to: slider.to * 2
                onMoved: slider.stepSize = value
            }
            Label {
                Kirigami.FormData.label: "Step size: "
                text: slider.stepSize
            }
        }
    }
}
