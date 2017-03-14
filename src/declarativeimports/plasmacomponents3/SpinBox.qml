

import QtQuick 2.6
import QtQuick.Controls 2.0
import QtQuick.Templates 2.0 as T
import org.kde.plasma.core 2.0 as PlasmaCore
import "private" as Private

T.SpinBox {
    id: control

    implicitWidth: Math.max(units.gridUnit * 6, contentItem.implicitWidth + 2 * padding + up.indicator.implicitWidth + down.indicator.implicitWidth)
    implicitHeight: units.gridUnit * 1.6

    padding: 6
    leftPadding: padding + height
    rightPadding: padding + height

    validator: IntValidator {
        locale: control.locale.name
        bottom: Math.min(control.from, control.to)
        top: Math.max(control.from, control.to)
    }

    contentItem: TextInput {
        text: control.textFromValue(control.value, control.locale)
        opacity: control.enabled ? 1 : 0.6

        font: control.font
        color: theme.viewTextColor
        selectionColor: theme.highlightColor
        selectedTextColor: theme.selectedTextColor
        horizontalAlignment: Qt.AlignHCenter
        verticalAlignment: Qt.AlignVCenter

        readOnly: !control.editable
        validator: control.validator
        inputMethodHints: Qt.ImhFormattedNumbersOnly
    }

    up.indicator: Item {
        x: control.mirrored ? 0 : parent.width - width
        implicitHeight: parent.height
        implicitWidth: implicitHeight
        PlasmaCore.FrameSvgItem {
            anchors {
                fill: parent
                margins: base.margins.right
            }
            imagePath: "widgets/button"
            prefix: up.pressed ? "pressed" : "normal"
            Label {
                anchors.centerIn: parent
                text: "+"
            }
        }
    }

    down.indicator:Item {
        x: control.mirrored ? parent.width - width : 0
        implicitHeight: parent.height
        implicitWidth: implicitHeight
        PlasmaCore.FrameSvgItem {
            anchors {
                fill: parent
                margins: base.margins.left
            }
            imagePath: "widgets/button"
            prefix: down.pressed ? "pressed" : "normal"
            Label {
                anchors.centerIn: parent
                text: "-"
            }
        }
    }

    background: Item {
        Private.TextFieldFocus {
            state: control.activeFocus ? "focus" : (control.hovered ? "hover" : "hidden")
            anchors.fill: parent
        }
        PlasmaCore.FrameSvgItem {
            id: base
            anchors.fill: parent
            imagePath: "widgets/lineedit"
            prefix: "base"
        }
    }
}
