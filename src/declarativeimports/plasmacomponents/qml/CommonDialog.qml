/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import "." 2.0 as PlasmaComponents

/**
 * CommonDialog is a convenience component that provides a dialog with the
 * platform-style title area. You only have to define titleText. CommonDialog
 * handles its layout automatically.
 *
 * Note: This component is experimental, so it may be changed or removed in
 * future releases.
 */
PlasmaComponents.Dialog {
    id: root

    /** type:string the title of the dialog */
    property alias titleText: titleAreaText.text

    /** the name or path of the dialog title icon */
    property string titleIcon

    /** the texts of all the buttons */
    property var buttonTexts: []

    /**
     * Emitted when the use clicks on a button
     * @param index the index of the clicked button: buttonTexts[index] will hold the text of the clicked button.
     */
    signal buttonClicked(int index)

    Accessible.role: Accessible.Dialog

    onButtonTextsChanged: {
        for (var i = buttonRow.children.length; i > 0; --i) {
            buttonRow.children[i - 1].destroy()
        }
        for (var j = 0; j < buttonTexts.length; ++j) {
            var button = buttonComponent.createObject(buttonRow)
            button.text = buttonTexts[j]
            button.index = j
        }
    }

    Component {
        id: buttonComponent
        PlasmaComponents.Button {
            property int index

            onClicked: {
                if (root.status == PlasmaComponents.DialogStatus.Open) {
                    root.buttonClicked(index)
                    root.close()
                }
            }
        }
    }

    QtObject {
        id: internal

        /*function buttonWidth() {
            switch (buttonTexts.length) {
                case 0: return 0
                case 1: return Math.round((800 - 3 * 4) / 2)
                default: return (buttonContainer.width - (buttonTexts.length + 1) *
                    4) / buttonTexts.length
            }
        }*/

        function iconSource() {
            return root.titleIcon
        }
    }

    title: PlasmaCore.FrameSvgItem {
        imagePath: "widgets/extender-dragger"
        prefix: "root"
        visible: titleAreaText.text != ""
        anchors.left: parent.left
        anchors.right: parent.right
        //FIXME: +5 because of Plasma::Dialog margins
        height: titleAreaText.paintedHeight + margins.top + margins.bottom

        LayoutMirroring.childrenInherit: true

        Column {
            id: titleLayoutHelper // needed to make the text mirror correctly

            anchors {
                right: parent.right
                left: titleAreaIcon.source == "" ? parent.left : titleAreaIcon.right
                top: parent.top
                bottom: parent.bottom
                leftMargin: parent.margins.left
                rightMargin: parent.margins.right
                topMargin: parent.margins.top
                bottomMargin: parent.margins.bottom
            }

            PlasmaComponents.Label {
                id: titleAreaText
                LayoutMirroring.enabled: root.LayoutMirroring.enabled
                elide: Text.ElideRight
                height: paintedHeight
                anchors {
                    left: parent.left
                    right: parent.right
                }
                horizontalAlignment: Text.AlignHCenter
                verticalAlignment: Text.AlignVCenter
            }
        }

        PlasmaCore.IconItem {
            id: titleAreaIcon
            width: PlasmaCore.Units.iconSizes.small
            height: PlasmaCore.Units.iconSizes.small
            source: titleIcon
            anchors.left: parent.left
            anchors.rightMargin: 4
            anchors.verticalCenter: parent.verticalCenter
        }
    }

    buttons: Row {
        id: buttonRow

        LayoutMirroring.enabled: false
        LayoutMirroring.childrenInherit: true
        objectName: "buttonRow"
        anchors.centerIn: parent
        spacing: 4
    }
}
