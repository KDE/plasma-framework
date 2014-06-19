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

import QtQuick.Controls 1.1 as QtQuickControls
import "styles" as Styles

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
}
