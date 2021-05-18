/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PC2
import org.kde.plasma.components 3.0 as PlasmaComponents

 /**
  * A standard basic header for plasmoids which has title, a config button and
  * a popup menu with all extra plasmoid actions.
  * By default, it will be invisible when the plamsoid is in the system tray, as it provides a replacement header with the same features
  * 
  * 
  * @inherit PlasmoidHeading
  */
PlasmoidHeading {
    /**
     * extraControls: list<QtObject>
     * any extra control and button that may be inserted in the  heading
     */
    default property alias extraControls: extraControlsLayout.data

    visible: !(plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)

    contentItem: RowLayout {
        Heading {
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
            Layout.fillWidth: true
            visible: (plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)
            level: 1
            text: plasmoid.title
        }
        RowLayout {
            id: extraControlsLayout
            visible: children.length > 0
            Layout.fillHeight: true
        }
        PlasmaComponents.ToolButton {
            id: actionsButton
            visible: visibleActions > 0 && !(plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)
            checked: configMenu.status !== PC2.DialogStatus.Closed
            property int visibleActions: 0
            property QtObject singleAction

            Component.onCompleted: updateVisibleActions()
            function updateVisibleActions() {
                let newSingleAction = null;
                let newVisibleActions = 0;
                for (let i in plasmoid.contextualActions) {
                    let action = plasmoid.contextualActions[i];
                    if (action.visible && action !== plasmoid.action("configure")) {
                        newVisibleActions++;
                        newSingleAction = action;
                        action.changed.connect(() => {updateVisibleActions()});
                    }
                }
                if (newVisibleActions > 1) {
                    newSingleAction = null;
                }
                visibleActions = newVisibleActions;
                singleAction = newSingleAction;
            }
            Connections {
                target: plasmoid
                function onContextualActionsChanged() {updateVisibleActions();}
            }
            icon.name: "application-menu"
            checkable: visibleActions > 1
            contentItem.opacity: visibleActions > 1
            // NOTE: it needs an IconItem because QtQuickControls2 buttons cannot load QIcons as their icon
            PlasmaCore.IconItem {
                parent: actionsButton
                anchors.centerIn: parent
                active: actionsButton.hovered
                implicitWidth: PlasmaCore.Units.iconSizes.smallMedium
                implicitHeight: implicitWidth
                source: actionsButton.singleAction !== null ? actionsButton.singleAction.icon : ""
                visible: actionsButton.singleAction
            }
            onToggled: {
                if (checked) {
                    configMenu.openRelative();
                } else {
                    configMenu.close();
                }
            }
            onClicked: {
                if (singleAction) {
                    singleAction.trigger();
                }
            }
            PlasmaComponents.ToolTip {
                text: actionsButton.singleAction ? actionsButton.singleAction.text : i18nd("libplasma5", "More actions")
            }
            PC2.Menu {
                id: configMenu
                visualParent: actionsButton
                placement: PlasmaCore.Types.BottomPosedLeftAlignedPopup
            }

            Instantiator {
                model: plasmoid.contextualActions
                delegate: PC2.MenuItem {
                    id: menuItem
                    action: modelData
                }
                onObjectAdded: {
                    if (object.action !== plasmoid.action("configure")) {
                        configMenu.addMenuItem(object);
                    }
                }
            }
        }
        PlasmaComponents.ToolButton {
            icon.name: "configure"
            visible: plasmoid && plasmoid.action("configure") && !(plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)
            PlasmaComponents.ToolTip {
                text: parent.visible ? plasmoid.action("configure").text : ""
            }
            onClicked: plasmoid.action("configure").trigger();
        }
    }
}
