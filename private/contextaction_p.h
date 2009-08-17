/*
 *   Copyright 2008 by Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 by Petri Damsten <damu@iki.fi>
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

#ifndef PLASMA_CONTEXTACTIONPRIVATE_H
#define PLASMA_CONTEXTACTIONPRIVATE_H

#include "plasma/private/dataengineconsumer_p.h"

namespace Plasma
{

class ContextActionPrivate : public DataEngineConsumer
{
public:
    ContextActionPrivate(KService::Ptr service, ContextAction *contextAction) :
        q(contextAction),
        containment(0),
        contextActionDescription(service),
        initialized(false),
        needsConfig(false),
        hasConfig(false)
    {
    };

    static PackageStructure::Ptr s_packageStructure;

    ContextAction *q;
    Containment *containment;
    KPluginInfo contextActionDescription;
    Package *package;
    KServiceAction mode;
    bool initialized : 1;
    bool needsConfig : 1;
    bool hasConfig : 1;
};

} // namespace Plasma
#endif

