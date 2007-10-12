/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <kapplication.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <KLocale>
#include <KDebug>
#include <QIcon>

#include <KAction>
#include <KStandardAction>

#include "kcategorizeditemsview.h"
#include "appletbrowserwindow.h"

int main(int argc, char **argv)
{
    KLocale::setMainCatalog("konqueror");

    KAboutData * aboutData = new KAboutData(QByteArray("testgui"), QByteArray(""), ki18n("TEST"), QByteArray("0"), ki18n("..."), KAboutData::License_GPL, KLocalizedString());
    KCmdLineArgs::init(argc, argv, aboutData);

    KApplication test_Application;
    
    (new AppletBrowserWindow())->show();

    return test_Application.exec();
}
