/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.15
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.kirigami 2.15 as Kirigami

T.ScrollBar {
    id: controlRoot

    implicitWidth: Math.max(implicitBackgroundWidth + leftInset + rightInset,
                            implicitContentWidth + leftPadding + rightPadding)
    implicitHeight: Math.max(implicitBackgroundHeight + topInset + bottomInset,
                             implicitContentHeight + topPadding + bottomPadding)

    hoverEnabled: interactive

    visible: (size > 0 && size < 1 && policy === T.ScrollBar.AsNeeded) || policy === T.ScrollBar.AlwaysOn
    minimumSize: horizontal ? height / width : width / height

    // Working around weird default values for `margins` (== `fixedMargins`) and `inset` (== -1)
    // TODO KF6: Use 0 as the default value for inset and margins
    leftPadding: scrollbarSvg.hasElement(`${handle.usedPrefix}-hint-left-inset`) ? handle.inset.left : horizontalPadding
    rightPadding: scrollbarSvg.hasElement(`${handle.usedPrefix}-hint-right-inset`) ? handle.inset.right : horizontalPadding
    topPadding: scrollbarSvg.hasElement(`${handle.usedPrefix}-hint-top-inset`) ? handle.inset.top : verticalPadding
    bottomPadding: scrollbarSvg.hasElement(`${handle.usedPrefix}-hint-bottom-inset`) ? handle.inset.bottom : verticalPadding
    leftInset: scrollbarSvg.hasElement(`${bgFrame.usedPrefix}-hint-left-inset`) ? bgFrame.inset.left : 0
    rightInset: scrollbarSvg.hasElement(`${bgFrame.usedPrefix}-hint-right-inset`) ? bgFrame.inset.right : 0
    topInset: scrollbarSvg.hasElement(`${bgFrame.usedPrefix}-hint-top-inset`) ? bgFrame.inset.top : 0
    bottomInset: scrollbarSvg.hasElement(`${bgFrame.usedPrefix}-hint-bottom-inset`) ? bgFrame.inset.bottom : 0

    Rectangle {
        id: separator
        anchors.left: parent.left
        width: controlRoot.horizontal ? parent.width : undefined
        height: controlRoot.vertical ? parent.height : undefined
        // I'm wary of adding things that could be considered official features
        // of the theming system willy-nilly, so this hint is marked private.
        // Technically, there's nothing stopping theme authors from using this
        // anyway, but I don't want to have to support it long term until we're
        // sure we want this.
        visible: scrollbarSvg.hasElement("private-hint-show-separator")
            && controlRoot.interactive
            && (controlRoot.mirrored ? controlRoot.rightInset > 0 : controlRoot.leftInset > 0)
        implicitWidth: PlasmaCore.Units.devicePixelRatio
        implicitHeight: implicitWidth
        color: PlasmaCore.ColorScope.textColor
        opacity: 0.1
    }

    background: PlasmaCore.FrameSvgItem {
        id: bgFrame
        implicitWidth: Math.max(scrollbarSvg.elementSize("hint-scrollbar-size").width, fixedMargins.left + fixedMargins.right)
        implicitHeight: Math.max(scrollbarSvg.elementSize("hint-scrollbar-size").height, fixedMargins.top + fixedMargins.bottom)
        imagePath:"widgets/scrollbar"
        colorGroup: PlasmaCore.ColorScope.colorGroup
        prefix: controlRoot.horizontal ? "background-horizontal" : "background-vertical"
        opacity: controlRoot.hovered && controlRoot.interactive
        visible: opacity > 0
        Behavior on opacity {
            NumberAnimation {
                duration: PlasmaCore.Units.longDuration
                easing.type: Easing.OutCubic
            }
        }
    }

    contentItem: PlasmaCore.FrameSvgItem {
        id: handle
        imagePath:"widgets/scrollbar"
        implicitWidth: Math.max(scrollbarSvg.elementSize("hint-scrollbar-size").width, fixedMargins.left + fixedMargins.right)
        implicitHeight: Math.max(scrollbarSvg.elementSize("hint-scrollbar-size").height, fixedMargins.top + fixedMargins.bottom)
        colorGroup: PlasmaCore.ColorScope.colorGroup
        prefix: controlRoot.interactive && (controlRoot.pressed || controlRoot.hovered) && controlRoot.enabled ? "mouseover-slider" : "slider"
        opacity: enabled ? 1 : 0.5
    }

    PlasmaCore.Svg {
        id: scrollbarSvg
        imagePath: "widgets/scrollbar"
        //TODO: support arrows?
        property bool arrowPresent: scrollbarSvg.hasElement("arrow-up")
        //new theme may be different
        onRepaintNeeded: arrowPresent = scrollbarSvg.hasElement("arrow-up")
    }
}
