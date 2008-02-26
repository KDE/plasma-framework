/*
 * Copyright 2008 Aaron J. Seigo
 *
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
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

#include <QCoreApplication>

#include <KAboutData>
#include <KApplication>
#include <KCmdLineArgs>
#include <KComponentData>
#include <KConfig>
#include <KDebug>

#include "plasma/appletbrowser.h"

static const char description[] = I18N_NOOP("Applet browser dialog");
static const char version[] = "1.0";

int main(int argc, char *argv[])
{
    KAboutData aboutData("plasamappletbrowser", 0, ki18n("Plasma Applet Browser"),
                         version, ki18n( description ), KAboutData::License_GPL,
                         ki18n("(C) 2008, Aaron Seigo"));
    aboutData.addAuthor(ki18n("Aaron Seigo"), ki18n("Original author"), "aseigo@kde.org");
    KCmdLineArgs::init(argc, argv, &aboutData);
    KApplication app;

    Plasma::AppletBrowser br(0);
    br.setApplication(QString());
    br.show();

    return app.exec();
}

