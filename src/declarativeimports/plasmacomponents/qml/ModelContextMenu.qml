/*
 *   Copyright 2014 David Edmundson <davidedmundson@kde.org>
 *   Copyright 2017 Kai Uwe Broulik <kde@privat.broulik.de>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import org.kde.plasma.components 2.0 as PlasmaComponents

import QtQuick 2.1
import QtQml 2.0

/**
 * A ModelMenu creates a context menu with items populated from a model or a QList<QAction*>
 * For standard item models, actions are created using the following model role names or properties
 *  display - a string contains the action name
 *  decoration - an icon to display
 *  separator - boolean that will add a seperator in the list
 *
 *
 *
 * Example code:
 *
 * @code
 * ModelMenu {
 *     id: menu
 *     visualParent: someButton
 *     model: myModel
 * }
 *
 * Button {
 *      id: someButton
 *      onClicked: menu.popup()
 * }
 * @endcode
 */

PlasmaComponents.ContextMenu {
    id: menu

    /**
     * The model containing menu items
     */
    property alias model: instantiator.model

    /**
     * This signal is emitted when a menu item is clicked.
     * The attached model properties for that menu item are passed as an argument
     */
    signal clicked(var model)

    //ContextMenu cannot have child items, so in order to have ContextMenu as the root object of this item
    //we create a new property which contains an item which can then load the child items
    property Instantiator _children: Instantiator {
        id: instantiator
        delegate: PlasmaComponents.MenuItem {
            //for QList<QAction*> Repeater adds an attached property modelData
            //for QAbstractItemModel* it doesn't. Not checking causes errors
            text: (typeof(modelData) != "undefined" ? modelData.text : model.display) || ""
            icon: typeof(modelData) != "undefined" ? modelData.icon : model.decoration
            separator: (typeof(modelData) != "undefined" ? modelData.separator : model.separator === true) || false
            section: (typeof(modelData) != "undefined" ? modelData.section : model.section === true) || false
            onClicked: {
                menu.clicked(typeof(modelData) != "undefined" ? modelData : model)
            }
        }

        onObjectAdded: menu.addMenuItem(object)
        onObjectRemoved: menu.removeMenuItem(object)
    }
    Accessible.role: Accessible.PopupMenu
}
