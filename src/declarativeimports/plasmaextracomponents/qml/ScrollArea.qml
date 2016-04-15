/*
*   Copyright 2014 David Edmundson <davidedmundson@kde.org>
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
 */

QtQuickControls.ScrollView {
    id: root

    style: Styles.ScrollViewStyle{}

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
    //end hack
}
