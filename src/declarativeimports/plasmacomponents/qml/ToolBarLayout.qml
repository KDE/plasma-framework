/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
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
