/*
    SPDX-FileCopyrightText: 2012 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.1
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 2.0 as PlasmaComponents

Item {
    id: root
    property Item visualParent
    z: 9000
    property int status: PlasmaComponents.DialogStatus.Closed
    default property alias data: contentItem.data

    function open()
    {
        if (root.visualParent) {
            internal.parentPos = root.visualParent.mapToItem(dismissArea.parent, 0, 0)
        }
        root.status = PlasmaComponents.DialogStatus.Opening
        appearAnimation.running = true
    }

    function close()
    {
        root.status = PlasmaComponents.DialogStatus.Closing
        appearAnimation.running = true
    }

    SequentialAnimation {
        id: appearAnimation
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
            easing.type: Easing.InOutQuad
            target: dismissArea
            properties: "opacity"
            to: root.status == PlasmaComponents.DialogStatus.Opening ? 1 : 0
        }
        ScriptAction {
            script: root.status == PlasmaComponents.DialogStatus.Opening ? root.status = PlasmaComponents.DialogStatus.Open : PlasmaComponents.DialogStatus.Closed
        }
    }

    MouseArea {
        id: dismissArea
        z: 9000
        anchors.fill: parent
        opacity: 0
        //to not pass hover events to the background
        hoverEnabled: true

        //FIXME: this is an hack: it's taking the dialog background making sure no opaque or transparent are selected
        //in Plasma2 we need to have the backingstore blur there as well
        PlasmaExtras.FallbackComponent {
            id: fallbackComponent
            basePath: "desktoptheme"
            candidates: [PlasmaCore.Theme.themeName, "default"]
            property string svgPath: fallbackComponent.filePath("/dialogs/background.svgz")
        }
        Connections {
            target: theme
            //fallback if inline-background doesn't work
            function onThemeChanged() {
                fallbackComponent.svgPath = fallbackComponent.filePath("/dialogs/background.svgz")
                shadowFrame.visible = backgroundSvg.hasElement("shadow-top")
            }
        }
        PlasmaCore.FrameSvgItem {
            id: shadowFrame
            imagePath: fallbackComponent.svgPath
            prefix: "shadow"
            anchors {
                fill: internal
                leftMargin: -margins.left
                topMargin: -margins.top
                rightMargin: -margins.right
                bottomMargin: -margins.bottom
            }
            Component.onCompleted: shadowFrame.visible = backgroundSvg.hasElement("shadow-top")
        }
        PlasmaCore.FrameSvgItem {
            id: internal
            property point parentPos
            imagePath: fallbackComponent.svgPath
            property bool under: root.visualParent ? internal.parentPos.y + root.visualParent.height + height < dismissArea.height : true
            //bindings won't work inside anchors definition
            onUnderChanged: {
                if (under) {
                    tipSvg.anchors.top = undefined
                    tipSvg.anchors.bottom = tipSvg.parent.top
                } else {
                    tipSvg.anchors.bottom = undefined
                    tipSvg.anchors.top = tipSvg.parent.bottom
                }
            }

            x: {
                if (root.visualParent) {
                    internal.parentPos.x - internal.width/2 + root.visualParent.width/2
                } else {
                    dismissArea.width/2 - internal.width/2
                }
            }
            y: {
                if (root.visualParent) {
                    if (under) {
                        internal.parentPos.y + root.visualParent.height
                    } else {
                        internal.parentPos.y - internal.height
                    }
                } else {
                    dismissArea.height/2 - internal.height/2
                }
            }
            width: contentItem.width + margins.left + margins.right
            height: contentItem.height + margins.top + margins.bottom

            PlasmaCore.SvgItem {
                id: tipSvg
                visible: root.visualParent != null
                svg: PlasmaCore.Svg {
                    id: backgroundSvg
                    imagePath: fallbackComponent.svgPath
                }
                elementId: internal.under ? "balloon-tip-top" : "balloon-tip-bottom"
                anchors {
                    horizontalCenter: parent.horizontalCenter
                    bottom: parent.top
                    top: parent.bottom
                    topMargin: -backgroundSvg.elementSize("hint-bottom-shadow").height - 1
                    bottomMargin: -backgroundSvg.elementSize("hint-top-shadow").height - 1
                }
                width: naturalSize.width
                height: naturalSize.height
            }
            MouseArea {
                id: contentItem
                x: parent.margins.left
                y: parent.margins.top
                width: childrenRect.width
                height: childrenRect.height
                onClicked: mouse.accepted = true
            }
        }
        onClicked: {
            root.close()
        }
        Component.onCompleted: {
            var candidate = root
            while (candidate.parent) {
                candidate = candidate.parent
            }
            if (candidate) {
                dismissArea.parent = candidate
            } else {
                dismissArea.visible = false
            }
            internal.parentPos = root.mapToItem(dismissArea.parent, 0, 0)
        }
    }
}
