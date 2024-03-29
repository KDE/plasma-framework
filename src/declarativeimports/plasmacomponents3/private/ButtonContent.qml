/*
 * SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.ksvg 1.0 as KSvg
import org.kde.kirigami 2 as Kirigami
import "../" as PlasmaComponents

GridLayout {
    id: root

    property string labelText: ""

    readonly property bool usingFocusBackground: !parent.flat && buttonSvg.hasElement("hint-focus-highlighted-background") && parent.visualFocus && !(parent.pressed || parent.checked)
    readonly property int defaultIconSize: parent.flat ? Kirigami.Units.iconSizes.smallMedium : Kirigami.Units.iconSizes.small

    columns: parent.display == T.Button.TextBesideIcon ? 2 : 1

    rowSpacing: parent.spacing
    columnSpacing: rowSpacing

    Kirigami.Icon {
        id: icon

        Layout.alignment: Qt.AlignCenter

        Layout.fillWidth: root.parent.display !== T.Button.TextBesideIcon || root.labelText.length === 0
        Layout.fillHeight: true

        Layout.minimumWidth: Math.min(root.width, root.height, implicitWidth)
        Layout.minimumHeight: Math.min(root.width, root.height, implicitHeight)

        Layout.maximumWidth: root.parent.icon.width > 0 ? root.parent.icon.width : Number.POSITIVE_INFINITY
        Layout.maximumHeight: root.parent.icon.height > 0 ? root.parent.icon.height : Number.POSITIVE_INFINITY

        implicitWidth: root.parent.icon.width > 0 ? root.parent.icon.width : root.defaultIconSize
        implicitHeight: root.parent.icon.height > 0 ? root.parent.icon.height : root.defaultIconSize
        visible: source.length > 0 && root.parent.display !== T.Button.TextOnly
        source: root.parent.icon ? (root.parent.icon.name || root.parent.icon.source) : ""
        selected: root.usingFocusBackground
    }
    PlasmaComponents.Label {
        id: label
        Layout.fillWidth: true
        Layout.fillHeight: true
        visible: text.length > 0 && root.parent.display !== T.Button.IconOnly
        text: root.labelText
        font: root.parent.font
        color: root.usingFocusBackground ? Kirigami.Theme.highlightedTextColor : Kirigami.Theme.textColor
        horizontalAlignment: root.parent.display !== T.Button.TextUnderIcon && icon.visible ? Text.AlignLeft : Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
    }
    KSvg.Svg {
        id: buttonSvg
        imagePath: "widgets/button"
    }
}
