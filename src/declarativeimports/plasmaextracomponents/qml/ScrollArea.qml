/*
    SPDX-FileCopyrightText: 2014 David Edmundson <davidedmundson@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.2
import QtQuick.Controls 1.1 as QtQuickControls
import QtQuick.Controls.Styles.Plasma 2.0 as Styles

/**
 * This item takes a Flickable and automatically puts scrollbars in adjusting
 * the layout if needed. The scrollbars will be interactive or not, depending
 * on the platform. If flickableItem is a categorized ListView the vertical
 * scrollbar will be a SectionScroller.
 *
 * See QtControls.ScrollView for full API
 * TODO: soon to be deprecated after we can use QtQuickControls2 SvrollView instead (Qt 5.10)
 */

QtQuickControls.ScrollView {
    id: root

    style: Styles.ScrollViewStyle{}
    frameVisible: true
    activeFocusOnTab: false

    //START HACK
    //The following is a workaround for QTBUG-17051
    //Scrollview disables interactive property on the listview for non-touch screen devices
    //which would make sense except this also breaks keyboard interaction which is a worse problem

    //this is fixed in 5.7 where interactive is split into two properties

    onContentItemChanged: {
        if (contentItem.interactive !== undefined) {
            contentItem.interactive = true;
        }
        contentItem.focus = true
    }
    //we want plasmoid popups open as quick as possible at start.
    //this makes sure things with big list views (kickoff, clipboard)
    //are loaded when created, even before they are shown
    Component.onCompleted: {
        if (typeof contentItem.forceLayout === "function") {
            contentItem.forceLayout();
        }
    }
    //end hack
}
