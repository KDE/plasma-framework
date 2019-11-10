import QtQuick 2.0

import org.kde.plasma.components 2.0 as PlasmaComponents

import QtQuick.Controls 1.3

Rectangle {
    id: root
    color: "white"
    width: 800
    height: 300

    property string longText: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed at volutpat nibh, non elementum nulla. Nunc sem magna, semper sit amet sollicitudin et, vestibulum sed metus. Fusce tempor dolor purus, non posuere urna sodales in. Aenean eu erat ipsum. Fusce egestas pulvinar nisi. Mauris vel enim tincidunt, elementum diam sed, tincidunt nulla. Maecenas tempus vitae ligula et convallis. Nullam justo velit, dignissim a nisl at, blandit posuere leo. Maecenas ac scelerisque odio, eget placerat ipsum. Ut iaculis, tortor et ullamcorper fringilla, mauris neque dapibus arcu, eget suscipit libero libero ut nunc. Sed maximus enim a ligula facilisis, non efficitur dolor blandit. Curabitur venenatis mattis erat ac gravida."

    Flow {
        anchors.fill: parent
        anchors.margins: 20
        spacing: 20

        PlasmaComponents.TextArea {
            placeholderText: "CHEESE"
            width: 150
            height: 100
        }

        PlasmaComponents.TextArea {
            text: root.longText
            width: 150
            height: 100
        }


        PlasmaComponents.TextArea {
            text: root.longText
            wrapMode: Text.Wrap
            width: 150
            height: 100
        }
    }
}
