/*
 *   Copyright (C) 2005 by Aaron Seigo <aseigo@kde.org>
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "applet.h"
#include "appletChain.h"

namespace Plasma
{

class AppletChain::Private
{
    public:
        Private()
            : popupDirection(Up),
              constraint(Plasma::NoConstraint),
              screenEdge(BottomEdge)
        {
        }


        Direction popupDirection;
        AppletConstraint constraint;
        ScreenEdge screenEdge;
        Applet::List applets;

        // configuration items kept in the display widget:
        //  - start coordinates and length for docked chain
        //  - dimensions for floating chain
        //  - display order of applets
};

AppletChain::AppletChain(QObject* parent)
    : QObject(parent),
      d(new Private())
{
}

AppletChain::~AppletChain()
{
}

Plasma::AppletConstraint AppletChain::constraint()
{
    return d->constraint;
}

void AppletChain::setConstraint(Plasma::AppletConstraint constraint)
{
    if (d->constraint == constraint)
    {
        return;
    }

    d->constraint = constraint;

    foreach (Applet* applet, d->applets)
    {
        applet->constraintsUpdated();
    }
}

void AppletChain::setScreenEdge(Plasma::ScreenEdge edge)
{
    d->screenEdge = edge;
}

Plasma::ScreenEdge AppletChain::screenEdge()
{
    return d->screenEdge;
}

Plasma::Direction AppletChain::popupDirection() const
{
    return Plasma::edgeToPopupDirection(d->screenEdge);
}

} // Plasma namespace

#include "appletChain.moc"
