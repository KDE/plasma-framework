/*
    SPDX-FileCopyrightText: %{CURRENT_YEAR} %{AUTHOR} <%{EMAIL}>
    SPDX-License-Identifier: LGPL-2.1-or-later
*/

import QtQuick 2.1
import QtQuick.Layouts 1.0
import QtQuick.Controls 1.0 as QtControls

// for "units"
import org.kde.plasma.core 2.0 as PlasmaCore


ColumnLayout {
    id: root
    property alias cfg_DisplayText: textField.text

    RowLayout {
        spacing: PlasmaCore.Units.largeSpacing / 2

        // To allow aligned integration in the settings form,
        // "formAlignment" is a property injected by the config containment
        // which defines the offset of the value fields
        QtControls.Label {
            Layout.minimumWidth: width
            Layout.maximumWidth: width
            width: formAlignment - PlasmaCore.Units.largeSpacing
            horizontalAlignment: Text.AlignRight

            // use i18nd in config QML, as the default textdomain is set to that of the config container
            text: i18nd("plasma_wallpaper_org.kde.plasma.%{APPNAMELC}", "Text to Display:")
        }
        QtControls.TextField {
            id: textField
            Layout.fillWidth: true
        }
    }

    Item { // tighten layout
        Layout.fillHeight: true
    }
}
