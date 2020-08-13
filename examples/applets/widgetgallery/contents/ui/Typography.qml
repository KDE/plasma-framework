/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras


PlasmaComponents.Page {
    implicitWidth: childrenRect.width
    implicitHeight: childrenRect.height
    tools: Row {
        spacing: 5
        PlasmaComponents.ToolButton {
            visible: pageStack.depth > 1
            iconSource: "go-previous"
            onClicked: pageStack.pop()
        }
        PlasmaExtras.Heading {
            level: 1
            text: "Typography"
        }
    }

    PlasmaExtras.ScrollArea {
        anchors.fill: parent
        Flickable {
            id: flickable
            //contentWidth: column.width
            contentHeight: column.height
            clip: true
            anchors.fill: parent

            Column {
                id: column
                width: parent.width
                anchors {
                    //fill: parent
                    margins: 12
                }
                spacing: 12

                PlasmaExtras.Heading {
                    level: 1
                    text: "A Title"
                }

                Repeater {
                    model: 5
                    PlasmaExtras.Heading {
                        level: index + 1
                        text: "Header level " + (index + 1)
                    }
                }

                PlasmaExtras.Heading {
                    level: 1
                    text: "Paragraphs"
                }

                PlasmaExtras.Paragraph {
                    text: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Aenean sit amet turpis eros, in luctus lectus. Curabitur pulvinar ligula at leo pellentesque non faucibus mauris elementum. Pellentesque convallis porttitor sodales. Maecenas risus erat, viverra blandit vestibulum eu, suscipit in est. Praesent quis mattis eros. Sed ante ante, adipiscing non gravida sed, ultrices ultrices urna. Etiam congue mattis convallis. Maecenas sollicitudin mauris at lorem aliquam in venenatis erat convallis. Fusce eleifend scelerisque porttitor. Praesent metus sapien, hendrerit ac congue eget, feugiat id enim. Morbi venenatis gravida felis, vitae varius nunc dictum a. Etiam accumsan, velit ac tempor convallis, leo nibh consequat purus, sit amet fringilla nisi mi et libero."
                }

                PlasmaExtras.Paragraph {
                    text: "Donec tincidunt justo eget nulla eleifend hendrerit. Ut eleifend erat nibh. Nunc sagittis tincidunt quam quis lobortis. Nam suscipit ultrices nulla. Suspendisse ullamcorper eleifend massa eu pharetra. Donec tempor iaculis elit, in suscipit velit tristique eu. Curabitur suscipit, lacus id pharetra dapibus, velit ante consectetur erat, ac dignissim quam arcu vitae diam. Suspendisse laoreet tortor nec dolor venenatis ultricies. Sed nunc erat, placerat non gravida sit amet, ullamcorper eu turpis. Nullam vestibulum lacus sed tellus fermentum mollis id at urna. Sed eleifend lobortis mollis. Donec lacus dolor, varius commodo gravida et, fringilla in justo. Nam gravida lorem in odio viverra elementum. Suspendisse non tellus at justo convallis placerat vel ac tellus. Nulla tristique tristique dui ut vestibulum."
                }


                PlasmaExtras.Heading {
                    level: 1
                    text: "TBD Components"
                }

                PlasmaComponents.Label {
                    id: menuEntry
                    text: "Menu entry"
                }

                PlasmaComponents.Label {
                    id: notificationHeader
                    text: "Notification header"
                }

                PlasmaComponents.Label {
                    id: notificationContent
                    text: "Menu entry"
                }

                PlasmaComponents.Label {
                    id: activeTab
                    text: "Active tab"
                }

                PlasmaComponents.Label {
                    id: inactiveTab
                    text: "Intactive tab"
                }

                PlasmaComponents.Label {
                    id: activeSelection
                    text: "Active selection"
                }

                PlasmaComponents.Label {
                    id: inactiveSelection
                    text: "Inactive selection"
                }

                PlasmaComponents.Label {
                    id: listItemPrimary
                    text: "List item: primary text"
                }

                PlasmaComponents.Label {
                    id: listItemSub
                    text: "List item: secondary text"
                }

                PlasmaComponents.Label {
                    id:  listItemPrimarySelected
                    text: "List item: selected, primary text"
                }

                PlasmaComponents.Label {
                    id: listItemPrimarySub
                    text: "List item: selected, secondary text"
                }

                PlasmaComponents.Label {
                    id: widgetHeader
                    text: "Widget headers"
                }

                PlasmaComponents.Label {
                    id: widgetContent
                    text: "Widget Content"
                }

                PlasmaComponents.Label {
                    id: iconTextSelected
                    text: "Icon text, selected"
                }

                PlasmaComponents.Label {
                    id: iconTextUnselected
                    text: "Icon text, unselected"
                }

                PlasmaComponents.Label {
                    id: groupHeader
                    text: "Group Header"
                }

                PlasmaComponents.Label {
                    id: groupContent
                    text: "Group Content"
                }
            }
        }
    }
}

