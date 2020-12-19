/*
    SPDX-FileCopyrightText: 2011 Nokia Corporation and /or its subsidiary(-ies) <qt-info@nokia.com>

    This file is part of the Qt Components project.

    SPDX-License-Identifier: BSD-3-Clause
*/

import QtQuick 2.1
import "private/SectionScroller.js" as Sections
import org.kde.plasma.core 2.0 as PlasmaCore
import "." 2.0 as PlasmaComponents

/**
 * Similar to a ScrollBar or a ScrollDecorator.
 *
 * It's interactive and works on ListViews that have section.property set, so
 * its contents are categorized.
 *
 * An indicator will say to what category the user scrolled to. Useful for
 * things like address books or things sorted by date. Don't use with models
 * too big (thousands of items) because it implies loading all the items to
 * memory, as well loses precision.
 *
 * @inherit QtQuick.Item
 */
Item {
    id: root

    /**
     * The listview the sectionScroller will operate on. This component doesn't
     * work with Flickable or GridView.
     */
    property ListView listView

    onListViewChanged: {
        if (listView && listView.model)
            internal.initDirtyObserver();
    }

    Connections {
        target: listView
        function onModelChanged() {
            if (listView && listView.model) {
                internal.initDirtyObserver()
            }
        }
    }

    implicitWidth: scrollBar.implicitWidth
    Behavior on opacity {
        NumberAnimation {
            duration: PlasmaCore.Units.longDuration
        }
    }

    anchors {
        right: listView.right
        top: listView.top
        bottom: listView.bottom
    }


    PlasmaComponents.RangeModel {
        id: range

        minimumValue: 0
        maximumValue: Math.max(0, listView.contentHeight - listView.height)
        stepSize: 0
        //inverted: true
        positionAtMinimum: root.width*2
        positionAtMaximum: root.height - root.width*2
        value: listView.contentY
        onPositionChanged: {
            var section = Sections.closestSection(position/listView.height);
            if (section) {
                if (listView.section.criteria == ViewSection.FirstCharacter) {
                    sectionLabel.text = section[0];
                } else {
                    sectionLabel.text = section;
                }
            }
        }

    }

    PlasmaComponents.ScrollBar {
        id: scrollBar
        flickableItem: listView
        anchors.fill: parent
        interactive: true
    }
    PlasmaCore.FrameSvgItem {
        id: tooltip
        imagePath: "widgets/tooltip"
        width: sectionLabel.width + margins.left + margins.right
        height: sectionLabel.height + margins.top + margins.bottom
        Label {
            id: sectionLabel
            x: parent.margins.left
            y: parent.margins.top
        }
        y: Math.min(root.height-height-scrollBar.width, Math.max(scrollBar.width, range.position - height/2))
        anchors {
            //verticalCenter: handle.verticalCenter
            right: parent.left
        }
        opacity: sectionLabel.text && scrollBar.pressed ? 1 : 0
        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
            }
        }
    }


    Timer {
        id: dirtyTimer
        interval: 250
        onTriggered: {
            Sections.initSectionData(listView);
            internal.modelDirty = false;
            tooltip.visible = Sections._sections.length > 1
        }
    }
    QtObject {
        id: internal

        property bool modelDirty: false
        function initDirtyObserver() {
            Sections.initSectionData(listView);
            tooltip.visible = Sections._sections.length > 1
            function dirtyObserver() {
                if (!internal.modelDirty) {
                    internal.modelDirty = true;
                    dirtyTimer.running = true;
                }
            }

            if (listView.model.countChanged)
                listView.model.countChanged.connect(dirtyObserver);

            if (listView.model.itemsChanged)
                listView.model.itemsChanged.connect(dirtyObserver);

            if (listView.model.itemsInserted)
                listView.model.itemsInserted.connect(dirtyObserver);

            if (listView.model.itemsMoved)
                listView.model.itemsMoved.connect(dirtyObserver);

            if (listView.model.itemsRemoved)
                listView.model.itemsRemoved.connect(dirtyObserver);
        }
    }
    Accessible.role: Accessible.ScrollBar
}
