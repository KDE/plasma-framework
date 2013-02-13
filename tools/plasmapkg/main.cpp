/*
 * Copyright 2008 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <iostream>
#include <iomanip>

#include <QDir>
#include <QDBusInterface>

//#include <KApplication>
//#include <KAboutData>
#include <kcmdlineargs.h>
#include <kdebug.h>
//#include <KLocale>
//#include <KPluginInfo>
#include <kservice.h>
#include <kservicetypetrader.h>
#include <kshell.h>
#include <kstandarddirs.h>
//#include <KSycocai>
#include <klocalizedstring.h>

#include <plasma/packagestructure.h>
#include <plasma/package.h>
//#include <plasma/packagemetadata.h>

#include "plasmapkg.h"

// static const char description[] = I18N_NOOP("Install, list, remove Plasma packages");
// static const char version[] = "0.2";

void output(const QString &msg)
{
    std::cout << msg.toLocal8Bit().constData() << std::endl;
}


int main(int argc, char **argv)
{
//     KAboutData aboutData("plasmapkg", 0, qi18n("Plasma Package Manager"),
//                          version, qi18n(description), KAboutData::License_GPL,
//                          qi18n("(C) 2008, Aaron Seigo"));
//     aboutData.addAuthor( qi18n("Aaron Seigo"),
//                          qi18n("Original author"),
//                         "aseigo@kde.org" );
//
//     KComponentData componentData(aboutData);
    QLocalizedString description = qi18n("Plasma Package Manager");

    const char version[] = "1.90";

    KCmdLineArgs::init(argc, argv, "plasmapkg", "plasmapkg", qi18n("Plasma Package Manager"), version, description);

    //KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("h");
    options.add("hash <path>", qi18nc("Do not translate <path>", "Generate a SHA1 hash for the package at <path>"));
    options.add("g");
    options.add("global", qi18n("For install or remove, operates on packages installed for all users."));
    options.add("t");
    options.add("type <type>",
                qi18nc("theme, wallpaper, etc. are keywords, but they may be translated, as both versions "
                       "are recognized by the application "
                       "(if translated, should be same as messages with 'package type' context below)",
                       "The type of package, e.g. theme, wallpaper, plasmoid, dataengine, runner, layout-template, etc."),
                "plasmoid");
    //options.add("s");
    options.add("i");
    options.add("install <path>", qi18nc("Do not translate <path>", "Install the package at <path>"));
    options.add("s");
    options.add("show <name>", qi18nc("Do not translate <name>", "Show information of package <name>"));
    options.add("u");
    options.add("upgrade <path>", qi18nc("Do not translate <path>", "Upgrade the package at <path>"));
    options.add("l");
    options.add("list", qi18n("List installed packages"));
    options.add("list-types", qi18n("lists all known Package types that can be installed"));
    options.add("r");
    options.add("remove <name>", qi18nc("Do not translate <name>", "Remove the package named <name>"));
    options.add("p");
    options.add("packageroot <path>", qi18n("Absolute path to the package root. If not supplied, then the standard data directories for this KDE session will be searched instead."));
    KCmdLineArgs::addCmdLineOptions( options );

    Plasma::PlasmaPkg app(argc, argv);
    //app.runMain();
    return app.exec();
}

