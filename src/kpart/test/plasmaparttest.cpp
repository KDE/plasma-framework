/***************************************************************************
 *   Copyright (C) 2014 by Marco Martin <mart@kde.org>                 *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "plasmaparttest.h"
#include "plasmaparttest.moc"

#include <QApplication>

#include <KService>

PlasmaPartTest::PlasmaPartTest()
    : KParts::MainWindow( )
{
   
    KService::Ptr service = KService::serviceByDesktopName("plasma-kpart");
    KPluginFactory* factory = 0;
    if (service) {
        factory = KPluginLoader(service->library()).factory();
    }

    if (!service || !factory) {
        qWarning() << "Could not find our Part!";
        qApp->quit();
        return;
    }

    m_part = factory->create<KParts::ReadOnlyPart>(this);
    setCentralWidget(m_part->widget());
    setupGUI();

    // apply the saved mainwindow settings, if any, and ask the mainwindow
    // to automatically save settings if changed: window size, toolbar
    // position, icon size, etc.
    setAutoSaveSettings();
}

PlasmaPartTest::~PlasmaPartTest()
{
}

