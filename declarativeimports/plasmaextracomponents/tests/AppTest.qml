// -*- coding: iso-8859-1 -*-
/*
 *   Copyright 2011 Sebastian Kügler <mart@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.extras 0.1 as PlasmaExtras

PlasmaExtras.App {
    id: app
    width: 800
    height: 600

    property string formFactor: "tablet"

    navigation: navigationItem
    content: contentItem

    Item {
        id: navigationItem
        width: navigationWidth + space
        height: parent.height
//         PlasmaExtras.Title {
//             text: "Navigation is zoo"
//             elide: Text.ElideMiddle
//             anchors { top: parent.top; left: parent.left; right: parent.right; }
//         }
        Column {
            id: navigationColumn
            anchors.fill: navigationItem
            width: navigationWidth
            PlasmaExtras.Title {
                text: "Navigation Pane."
                elide: Text.ElideMiddle
                //anchors { top: parent.top; left: parent.left; topMargin: 48; }
            }
        }
        PlasmaComponents.CheckBox {
            text: "Make big"
            width: navigationWidth
            checked: navigationWidth > 200
            anchors { left: parent.left; right: parent.right; bottom: parent.bottom; }
            onCheckedChanged: {
                var w = !checked ? 120 : 340;
                print(" new: " + w);
                navigationWidth = w;
            }
            //width: contentItem.width
        }
        Rectangle { color: "blue"; opacity: 0.3; anchors.fill: parent; }
    }

    Item {
        id: contentItem
        Column {
            id: contentColumn
            anchors.fill: contentItem
            PlasmaExtras.Title {
                text: "Content is zooopah cool, too."
                elide: Text.ElideMiddle
                //anchors { top: parent.top; left: parent.left; topMargin: 48; }
            }
        }
        Rectangle { color: "green"; opacity: 0.8; anchors.fill: contentColumn; }
    }

    onWidthChanged: {
        return;
        print("window changed");
        if (width < navigationWidth + 200) {
            setNavigationWidth(48);
        } else {
            setNavigationWidth(340);
        }
    }

}
