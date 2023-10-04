/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.kirigami 2 as Kirigami

 /**
  * A standard basic header for plasmoids which has title, a config button and
  * a popup menu with all extra plasmoid actions.
  *
  * By default, it will be invisible when the plasmoid is in the system tray,
  * as it provides a replacement header with the same features
  *
  * @inherit PlasmoidHeading
  */
PlasmoidHeading {
    /**
     * extraControls: list<QtObject>
     *
     * Any extra control and button that may be inserted in the heading
     */
    default property alias extraControls: extraControlsLayout.data

    visible: !(Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)

    contentItem: RowLayout {
        Heading {
            elide: Text.ElideRight
            wrapMode: Text.NoWrap
            Layout.fillWidth: true
            visible: (Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)
            level: 1
            text: Plasmoid.title
        }
        RowLayout {
            id: extraControlsLayout
            visible: children.length > 0
            Layout.fillHeight: true
        }
        PlasmaComponents.ToolButton {
            id: actionsButton
            visible: visibleActions > 0 && !(Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)
            checked: configMenu.status !== PlasmaExtras.Menu.Closed
            property int visibleActions: menuItemFactory.count
            property QtObject singleAction: visibleActions === 1 ? menuItemFactory.object.action : null
            icon.name: "open-menu-symbolic"
            checkable: visibleActions > 1
            contentItem.opacity: visibleActions > 1
            // NOTE: it needs an Icon because QtQuickControls2 buttons cannot load QIcons as their icon
            Kirigami.Icon {
                parent: actionsButton
                anchors.centerIn: parent
                active: actionsButton.hovered
                implicitWidth: Kirigami.Units.iconSizes.smallMedium
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
                text: actionsButton.singleAction ? actionsButton.singleAction.text : i18nd("libplasma6", "More actions")
            }
            PlasmaExtras.Menu {
                id: configMenu
                visualParent: actionsButton
                placement: PlasmaExtras.Menu.BottomPosedLeftAlignedPopup
            }

            Instantiator {
                id: menuItemFactory
                model: {
                    configMenu.clearMenuItems();
                    const actions = [];
                    const configureAction = Plasmoid.internalAction("configure");
                    for (const action of Plasmoid.contextualActions) {
                        if (action.visible && action !== configureAction) {
                            actions.push(action);
                        }
                    }
                    return actions;
                }
                delegate: PlasmaExtras.MenuItem {
                    id: menuItem
                    action: modelData
                }
                onObjectAdded: {
                    configMenu.addMenuItem(object);
                }
            }
        }
        PlasmaComponents.ToolButton {
            icon.name: "configure"
            visible: plasmoid && Plasmoid.internalAction("configure") && !(Plasmoid.containmentDisplayHints & PlasmaCore.Types.ContainmentDrawsPlasmoidHeading)
            PlasmaComponents.ToolTip {
                text: parent.visible ? Plasmoid.internalAction("configure").text : ""
            }
            onClicked: Plasmoid.internalAction("configure").trigger();
        }
    }
}
