/*
    SPDX-FileCopyrightText: 2020 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.12
import QtQuick.Layouts 1.12

import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents
import org.kde.plasma.components 2.0 as PlasmaExtras
import "private" as Private

 /**
  * Item to be used as root item for representations (full and compact) of plasmoids.
  * It's a QtQuickControls2 Page, and like that one, has an header, a contentItem and a Footer
  * It may go over the plasmoid edges (both on desktop and popups) with the properties applyHorizontalPadding and applyVerticalPadding.
  * When the contentItem is a ScrollView or a Scrollarea, the plasmoid margins will be automatically removed.
  *
  * This code will create a full representation with a listview that will automatically 
  * full the whole area without margins from the plasmoid popup borders
  * @code
  * Plasmoid.Representation: PlasmaExtras.Representation {
  *     header: PlasmaExtras.BasicPlasmoidHeading{}
  *     contentItem: PlasmaComponent.ScrollView {
  *         ListView {
  *             ....
  *         }
  *     }
  * }
  * @endcode
  *
  * @since 5.77
  * @inherit QtQuick.Templates.Page
  */
 
 PlasmaComponents.Page {
    id: control

    // TODO KF6: should become possible to set the paddings directly (which won't be negative anymore)
    /**
     * collapseMarginsHint: bool
     * if true, the representation will remove any borders its container may have put and will be collapsed above its borders
     */
    property bool collapseMarginsHint: (control.contentItem instanceof PlasmaComponents.ScrollView) || (control.contentItem instanceof ScrollArea)

    leftPadding: backgroundMetrics.getMargin("left")
    rightPadding: backgroundMetrics.getMargin("right")
    topPadding: header && header.visible ? 0 : backgroundMetrics.getMargin("top")
    bottomPadding: footer && footer.visible ? 0 : backgroundMetrics.getMargin("bottom")

    Private.BackgroundMetrics {
        id: backgroundMetrics
        function getMargin(margin) {
            if (!hasInset) {
                return 0;
            } else {
                return control.collapseMarginsHint ? -backgroundMetrics.fixedMargins[margin] + backgroundMetrics.inset[margin] : 0
            }
        }
    }
 }
