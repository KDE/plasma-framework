/*
    SPDX-FileCopyrightText: 2018 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

import QtQuick 2.6
import QtQuick.Layouts 1.2
import QtQuick.Templates @QQC2_VERSION@ as T
import org.kde.kirigami 2 as Kirigami

T.PageIndicator {
    id: control

    implicitWidth: contentItem.implicitWidth + leftPadding + rightPadding
    implicitHeight: contentItem.implicitHeight + topPadding + bottomPadding

    padding: Kirigami.Units.smallSpacing
    spacing: Kirigami.Units.smallSpacing

    delegate: Rectangle {
        implicitWidth: Kirigami.Units.largeSpacing
        implicitHeight: implicitWidth

        radius: width
        color: Kirigami.Theme.textColor

        opacity: index === currentIndex ? 0.9 : pressed ? 0.7 : 0.5
        Behavior on opacity {
            OpacityAnimator {
                duration: Kirigami.Units.longDuration
                easing.type: Easing.InOutQuad
            }
        }
    }

    contentItem: Row {
        spacing: control.spacing

        Repeater {
            model: control.count
            delegate: control.delegate
        }
    }
}
