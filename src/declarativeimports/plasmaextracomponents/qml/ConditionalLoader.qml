/*
    Copyright (C) 2012 Marco Martin <mart@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 2.010-1301 USA.
*/
import QtQuick 2.0

/**
 * This item can load any qml component, just like a Loader. Important
 * difference, the component will only be loaded when the "when" property is
 * satisfied (ie evaluates to true) in this way it's easy to have big (and
 * memory expensive) parts of the user interface load only when a certain
 * condition is satisfied.  For instance the contents of the tabs of a TabBar
 * can be loaded only when they become the current page.
 * @deprecated use a Loader instead
 */
FocusScope {
    id: root

    /**
     * type:bool
     * Condition that tells when to load the declarative component
     */
    property alias when: loader.when

    /**
     * type:variant
     * It can be a string with a path name of a qml file or a Component. It's
     * the component that will be loaded when "when" is true. If the component
     * changes the old instantiated component will be deleted and the new one
     * will be loaded instead.
     */
    property alias source: loader.conditionalSource

    /**
     * type:Item
     * The item instantiated from component, if any.
     */
    property alias item: loader.item


    implicitWidth: loader.item ? loader.item.implicitWidth : 0
    implicitHeight: loader.item ? loader.item.implicitHeight : 0

    Loader {
        id: loader
        anchors.fill: parent
        focus: true

        property bool when: false
        property variant conditionalSource

        //internal
        property variant oldConditionalSource

        onWhenChanged: loadTimer.restart()
        onConditionalSourceChanged: loadTimer.restart()

        onLoaded: {
            loader.oldConditionalSource = loader.conditionalSource
            loader.item.visible = true
            loader.item.anchors.fill = loader.item.parent
            loadTimer.stop()
        }
        Timer {
            id: loadTimer
            interval: 0

            onTriggered: {
                if (loader.when &&
                    (loader.item === null ||
                     loader.conditionalSource !== loader.oldConditionalSource)) {
                    if (typeof(loader.conditionalSource) === "string") {
                        loader.source = loader.conditionalSource
                    } else {
                        loader.sourceComponent = loader.conditionalSource
                    }
                }
            }
        }
    }
}
