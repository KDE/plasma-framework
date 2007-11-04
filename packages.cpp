/******************************************************************************
*   Copyright 2007 by Aaron Seigo <aseigo@kde.org>                        *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

#include "packages_p.h"

#include <KLocale>

namespace Plasma
{

PlasmoidStructure::PlasmoidStructure()
    : Plasma::PackageStructure(QString("Plasmoid"))
{
    addDirectoryDefinition("images", "images", i18n("Images"));
    QStringList mimetypes;
    mimetypes << "image/svg+xml" << "image/png" << "image/jpeg";
    setMimetypes("images", mimetypes);

    addDirectoryDefinition("config", "config/xml", i18n("Configuration Definitions"));
    addDirectoryDefinition("configui", "config/ui", i18n("Configuration UI"));
    mimetypes.clear();
    mimetypes << "text/xml";
    setMimetypes("config", mimetypes);
    setMimetypes("configui", mimetypes);

    addDirectoryDefinition("scripts", "code", i18n("Executable Scripts"));
    mimetypes.clear();
    mimetypes << "text/*";
    setMimetypes("scripts", mimetypes);

    addFileDefinition("mainconfiggui", "config/ui/main.ui", i18n("Main Config UI File"));
    addFileDefinition("mainconfigxml", "config/xml/main.xml", i18n("Configuration XML file"));
    addFileDefinition("mainscript", "code/main", i18n("Main Script File"));
    setRequired("mainscript", true);
}

ThemePackageStructure::ThemePackageStructure()
    : Plasma::PackageStructure(QString("Plasma Theme"))
{
    addDirectoryDefinition("dialogs", "dialogs/", i18n("Images for dialogs"));
    addFileDefinition("dialogs/background", "dialogs/background.svg",
                      i18n("Generic dialog background"));
    addFileDefinition("dialogs/shutdowndlg", "dialogs/shutdowndlg.svg",
                      i18n("Background for the log out dialog"));
    addFileDefinition("dialogs/shutdowndlgbuttonglow", "dialogs/shutdowndlgbuttonglow.svg",
                      i18n("Overlay for buttons on the log out dialog"));

    addDirectoryDefinition("widgets", "widgets/", i18n("Images for widgets"));
    addFileDefinition("widgets/background", "widgets/background.svg",
                      i18n("Background image for plasmoids"));
    addFileDefinition("widgets/clock", "widgets/clock.svg",
                      i18n("Analog clock face"));
    addFileDefinition("widgets/iconbutton", "widgets/iconbutton.svg",
                      i18n("Back- and foregrounds for clickable icons"));
    addFileDefinition("widgets/plot-background", "widgets/plot-background.svg",
                      i18n("Background for graphing widgets"));

    addFileDefinition("colors", "colors", i18n("KColorScheme configuration file"));

    QStringList mimetypes;
    mimetypes << "image/svg+xml";
    setDefaultMimetypes(mimetypes);
}

} // namespace Plasma


