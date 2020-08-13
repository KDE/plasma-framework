/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

import QtQuick 2.0
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.components 2.0 as PlasmaComponents
import org.kde.plasma.extras 2.0 as PlasmaExtras
import org.kde.kquickcontrolsaddons 2.0 as KQuickControlsAddons

// PlasmoidPage

PlasmaComponents.Menu {
    id: testMenu

    PlasmaComponents.MenuItem {
        text: "Red Snapper"
        icon: "dragonplayer"
        onClicked: print(" Clicked on : " + text)
    }

    PlasmaComponents.MenuItem {
        text: "Eel"
        icon: "kthesaurus"
        onClicked: print(" Clicked on : " + text)
    }

    PlasmaComponents.MenuItem {
        text: "White Tip Reef Shark"
        icon: "kmag"
        onClicked: print(" Clicked on : " + text)
    }

    Component.onCompleted:{
        print("TestMenu.qml served .. opening");

    }
}

