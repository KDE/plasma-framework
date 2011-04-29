/* Copyright 2011 Kevin Ottens <ervin@kde.org>

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as
   published by the Free Software Foundation; either version 2,
   or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details

   You should have received a copy of the GNU Library General Public
   License along with this program; if not, write to the
   Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <iostream>

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusReply>

#include <KDE/KApplication>
#include <KDE/KAboutData>
#include <KDE/KCmdLineArgs>

static const char description[] = I18N_NOOP("Trigger the installation of a remove Plasma Widget");
static const char version[] = "0.1";

int addRemotePlasmoidToShell(const QString &shellName, const QString &url)
{
    QString serviceName = "org.kde." + shellName;
    QDBusInterface iface(serviceName, "/App");

    if (!iface.isValid()) {
        std::cerr << "Error: Couldn't contact "
                  << shellName.toLocal8Bit().constData() << std::endl;
        return 1;
    } else {
        QDBusReply<void> reply = iface.call("addRemotePlasmoid", url);
        if (!reply.isValid()) {
            std::cerr << "Error: Couldn't call addRemotePlasmoid on "
                      << shellName.toLocal8Bit().constData() << std::endl;
            return 1;
        }
    }

    return 0;
}

int main(int argc, char **argv)
{
    KAboutData aboutData("plasma-remote-helper", 0, ki18n("Plasma Remote Widget Helper"),
                         version, ki18n(description), KAboutData::License_GPL,
                         ki18n("(C) 2011 Kevin Ottens"));
    aboutData.addAuthor( ki18n("Kevin Ottens"),
                         ki18n("Original author"),
                        "ervin@kde.org" );

    KComponentData componentData(aboutData);

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("+<url>", ki18n("URL to the Plasma Remote Widget."));
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->count()<1) {
        KCmdLineArgs::usageError(i18n("Syntax Error: Not enough arguments"));
    } else if (args->count()>1) {
        KCmdLineArgs::usageError(i18n("Syntax Error: Too many arguments"));
    }

    QString url = args->arg(0);
    return addRemotePlasmoidToShell("plasma-desktop", url);
}

