/*
    SPDX-FileCopyrightText: 2011 Daker Fernandes Pinheiro <dakerfp@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import QtQuick.Window 2.2
import QtQuick.Controls 1.0
import QtQuick.Controls.Private 1.0 as QtQuickControlsPrivate
import org.kde.plasma.core 2.0 as PlasmaCore

/**
 * Private base component for several public components. Any element documented here
 * can be used in components inheriting from DualStateButton.
 */
Item {
    id: dualButton

    // Common API
    /**
     * If the button is checked, its checked property is true; otherwise false. The property is false by default.
     */
    property bool checked

    /**
     * type:bool
     * If the button is pressed, its pressed property is true.
     * @see clicked
     */
    property alias pressed: mouseArea.pressed

    /**
     * Emitted when the user clicked a mouse button over the button (or
     * tapped on the touch screen)
     */
    signal clicked()

    // Plasma API
    /**
     * The text is shown beside the button. By default text is an empty string.
     */
    property alias text: label.text // TODO: Not yet part of the common API
    property alias view: surfaceLoader.sourceComponent
    property alias shadow: shadowLoader.sourceComponent

    width: surfaceLoader.width + label.paintedWidth
    height: PlasmaCore.Theme.mSize(PlasmaCore.Theme.defaultFont).height*1.6
    // TODO: needs to define if there will be specific graphics for
    //     disabled buttons
    opacity: dualButton.enabled ? 1.0 : 0.5

    function released() {
        if (dualButton.enabled) {
            dualButton.checked = !dualButton.checked;
            dualButton.clicked();
        }
    }

    Keys.onReleased: {
        if(event.key == Qt.Key_Space ||
           event.key == Qt.Key_Return)
            released();
    }

    Loader {
        id: shadowLoader
        anchors.fill: surfaceLoader
        state: (dualButton.enabled && (dualButton.focus || mouseArea.containsMouse)) ? "hover" : "shadow"
    }

    Loader {
        id: surfaceLoader

        anchors {
            verticalCenter: parent.verticalCenter
            left: text ? parent.left : undefined
            horizontalCenter: text ? undefined : parent.horizontalCenter
        }
    }

    Text {
        id: label

        text: dualButton.text

        // Work around Qt bug where NativeRendering breaks for non-integer scale factors
        // https://bugreports.qt.io/browse/QTBUG-70481
        renderType: QtQuickControlsPrivate.Settings.isMobile || Screen.devicePixelRatio % 1 !== 0 ? Text.QtRendering : Text.NativeRendering
        anchors {
            top: parent.top
            bottom: parent.bottom
            left: surfaceLoader.right
            right: parent.right
            //FIXME: see how this margin will be set
            leftMargin: height/4
        }
        color: PlasmaCore.Theme.textColor
        verticalAlignment: Text.AlignVCenter
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        hoverEnabled: true

        onPressed: dualButton.forceActiveFocus();
        onReleased: dualButton.released();
    }
    Accessible.name: text
    Accessible.checkable: checkable
    Accessible.checked: checked
    function accessiblePressAction() {
        dualButton.forceActiveFocus()
        dualButton.released()
    }
}
