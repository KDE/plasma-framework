/*
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

import QtQuick 2.1

import "." 2.0 as PlasmaComponents

/**
 * Defines the content of a piece of the user interface, it's meant to be
 * loaded by a PageStack or TabGroup element.  The typical use can be in small
 * plasmoids or  mobile devices where the whole screen is a series of
 * interchangeable and flickable pages, of which the user navigates across.
 *
 * @inherit QtQuick.Item
 */
Item {
    id: root

    /**
     * type:PageStatus::Status
     * The status of the page
     *
     * One of the following:
     *
     * - PageStatus.Inactive: the page is not visible
     * - PageStatus.Activating: the page is transitioning into becoming the active page
     * - PageStatus.Active: the page is the current active page
     * - PageStatus.Deactivating: the page is transitioning into becoming inactive
     */
    property int status: PlasmaComponents.PageStatus.Inactive

    /**
     * type:PageStack
     * The page stack that this page is owned by.
     */
    property Item pageStack

    /**
     * Sets the orientation for the Page
     */
    property int orientationLock: PlasmaComponents.PageOrientation.Automatic

    /**
     * Defines the toolbar contents for the page. If the page stack is set up
     * using a toolbar instance, it automatically shows the currently active
     * page's toolbar contents in the toolbar.
     *
     * The default value is null resulting in the page's toolbar to be
     * invisible when the page is active.
     */
    property Item tools: null

    visible: false

    width: visible && parent ? parent.width : internal.previousWidth
    height: visible && parent ? parent.height : internal.previousHeight

    onWidthChanged: internal.previousWidth = (visible ? width : internal.previousWidth)
    onHeightChanged: internal.previousHeight = (visible ? height : internal.previousHeight)

    // This is needed to make a parentless Page component visible in the Designer of QtCreator.
    // It's done here instead of binding the visible property because binding it to a script
    // block returning false would cause an element on the Page not to end up focused despite
    // specifying focus=true inside the active focus scope. The focus would be gained and lost
    // in createObject.
    Component.onCompleted: if (!parent) visible = true

    QtObject {
        id: internal
        property int previousWidth: 0
        property int previousHeight: 0
    }
}
