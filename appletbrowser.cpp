/*
 *   Copyright (C) 2007 Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2 (or,
 *   at your option, any later version) as published by the Free Software
 *   Foundation
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

#include <appletbrowser.h>
#include <appletbrowser/appletbrowserwindow_p.h>

#include <corona.h>
#include <containment.h>

namespace Plasma
{

class AppletBrowser::Private
{
public:
    Private(AppletBrowser *parent, Corona *corona)
        : q(parent), window(new AppletBrowserWindow(corona))
    {}

    Private(AppletBrowser *parent, Containment *containment)
        : q(parent), window(new AppletBrowserWindow(containment))
    {}

    ~Private()
    {
        delete window;
    }

    AppletBrowser *q;
    AppletBrowserWindow *window;
};

AppletBrowser::AppletBrowser(Corona * corona)
    : d(new Private(this, corona))
{
}

AppletBrowser::AppletBrowser(Containment * containment)
    : d(new Private(this, containment))
{
}

AppletBrowser::~AppletBrowser()
{
    delete d;
}

void AppletBrowser::show()
{
    if (d->window) {
        d->window->show();
    }
}

void AppletBrowser::hide()
{
    if (d->window) {
        d->window->hide();
    }
}


} // namespace Plasma
