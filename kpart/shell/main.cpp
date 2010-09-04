/*
*   Copyright 2010 Ryan Rix <ry@n.rix.si>
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU Library General Public License version 2 as
*   published by the Free Software Foundation
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

#include "containmentshell.h"
#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>

static const char version[] = "0.1";

int main(int argc, char** argv)
{
	KAboutData about("plasma-kpart-shell", 0, ki18n("Plasma KPart Shell"), version, ki18n("A KDE KPart Application"), KAboutData::License_GPL, ki18n("(C) 2010 Ryan Rix"), KLocalizedString(), 0, "ry@n.rix.si");
	about.addAuthor( ki18n("Ryan Rix"), KLocalizedString(), "ry@n.rix.si" );
	KCmdLineArgs::init(argc, argv, &about);

	KApplication app;

	// see if we are starting with session management
	if (app.isSessionRestored())
		RESTORE(ContainmentShell)
	else
	{
		ContainmentShell* widget = new ContainmentShell;
		widget->show();
	}

	return app.exec();
}
