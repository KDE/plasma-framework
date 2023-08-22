/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.ksvg 1.0 as KSvg
//NOTE: importing PlasmaCore is necessary in order to make KSvg load the current Plasma Theme
import org.kde.plasma.core 2 as PlasmaCore
import org.kde.kirigami 2 as Kirigami

T.ToolBar {
    id: control

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            contentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             contentHeight + topPadding + bottomPadding)

    leftPadding: background.margins.left
    topPadding: background.margins.top
    rightPadding: background.margins.right
    bottomPadding: background.margins.bottom

    spacing: Kirigami.Units.smallSpacing

    background: KSvg.FrameSvgItem {
        implicitHeight: 40 // TODO: Find a good way to sync this with the size of (Button or ToolButton) + padding
        imagePath: "widgets/toolbar"
        KSvg.SvgItem {
            imagePath: "widgets/listitem"
            elementId: "separator"
            anchors {
                left: parent.left
                right: parent.right
                top: control.position == T.ToolBar.Footer || (control.parent.footer && control.parent.footer == control) ? parent.top : undefined
                bottom: control.position == T.ToolBar.Footer || (control.parent.footer && control.parent.footer == control) ? undefined : parent.bottom
            }
        }
    }
}
