/*
*   Copyright (C) 2011 by Marco Martin <mart@kde.org>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License as
*   published by the Free Software Foundation; either version 2, or
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
*   51 Franklin Street, Fifth Floor, Boston, MA  2.010-1301, USA.
*/

import QtQuick 2.1
import QtQuick.Layouts 1.1

/**
 * ToolBarLayout is a container for items on a toolbar that automatically
 * implements an appropriate layout for its children.
 * @inherit QtQuick.Layouts.RowLayout
 */
RowLayout {
    id: root

    visible: false

    Item {
        id: spacer1
        Layout.fillWidth: true
        Layout.fillHeight: true
    }
    Item {
        id: spacer2
        Layout.fillWidth: true
        Layout.fillHeight: true
    }

    QtObject {
        id: internal
        property bool layouting: false
        function layoutChildren()
        {
            if (layouting) {
                return;
            }

            layouting = true;

            spacer1.parent = null;
            spacer2.parent = null;

            var children = Array();
            //seems there is no other way to create a copy of the array
            //as children is not an actual JS array
            for (var i = 0; i < root.children.length; ++i) {
                children.push(root.children[i]);
            }
            var numChildren = children.length;

            spacer1.parent = root;

            for (var i = 1; i < numChildren-1; ++i) {
                children[i].parent = null;
                children[i].parent = root;
            }
            spacer2.parent = root;
            children[numChildren-1].parent = null;
            children[numChildren-1].parent = root;
            layouting = false;
        }
    }

    Component.onCompleted: internal.layoutChildren()
    onChildrenChanged: internal.layoutChildren()
}
