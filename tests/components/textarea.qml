/*
 * SPDX-FileCopyrightText: 2014 David Edmundson <kde@davidedmundson.co.uk>
 * SPDX-FileCopyrightText: 2019 Yuri Chornoivan <yurchor@ukr.net>
 * SPDX-FileCopyrightText: 2020 Noah Davis <noahadvs@gmail.com>
 * SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
 */
import QtQuick 2.0
import QtQuick.Layouts 1.12
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents

ComponentBase {
    id: root
    title: "Plasma Components 2 TextArea"

    property string longText: "Lorem ipsum dolor sit amet, consectetur adipiscing elit. Sed at volutpat nibh, non elementum nulla. Nunc sem magna, semper sit amet sollicitudin et, vestibulum sed metus. Fusce tempor dolor purus, non posuere urna sodales in. Aenean eu erat ipsum. Fusce egestas pulvinar nisi. Mauris vel enim tincidunt, elementum diam sed, tincidunt nulla. Maecenas tempus vitae ligula et convallis. Nullam justo velit, dignissim a nisl at, blandit posuere leo. Maecenas ac scelerisque odio, eget placerat ipsum. Ut iaculis, tortor et ullamcorper fringilla, mauris neque dapibus arcu, eget suscipit libero libero ut nunc. Sed maximus enim a ligula facilisis, non efficitur dolor blandit. Curabitur venenatis mattis erat ac gravida."

    contentItem: Flow {
        spacing: PlasmaCore.Units.gridUnit

        PlasmaComponents.TextArea {
            placeholderText: "CHEESE"
            width: 150
            height: 100
        }

        PlasmaComponents.TextArea {
            text: root.longText
            width: 150
            height: 100
        }


        PlasmaComponents.TextArea {
            text: root.longText
            wrapMode: Text.Wrap
            width: 150
            height: 100
        }
    }
}
