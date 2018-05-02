/*
 *   Copyright (c) 2009 Chani Armitage <chani@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
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

#ifndef PLASMA_CONTAINMENTACTIONSPRIVATE_H
#define PLASMA_CONTAINMENTACTIONSPRIVATE_H

namespace Plasma
{

class ContainmentActionsPrivate
{
public:
    ContainmentActionsPrivate(KService::Ptr service, ContainmentActions *containmentActions) :
        q(containmentActions),
        containmentActionsDescription(service),
        package(0),
        containment(0)
    {
    }

    ContainmentActions *q;

    QString currentTrigger;
    KPluginInfo containmentActionsDescription;
    Package *package;
    KServiceAction mode;
    Containment *containment;
};

} // namespace Plasma
#endif //PLASMA_CONTAINMENTACTIONSPRIVATE_H

