/*
 *   Copyright 2012 by Sebastian Kügler <sebas@kde.org>
 
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

#include "plasmaextracomponentsplugin.h"

#include "appbackgroundprovider_p.h"
#include "resourceinstance.h"
#include "fallbackcomponent.h"

#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeEngine>


// #include <KSharedConfig>
// #include <KConfigGroup>

void PlasmaExtraComponentsPlugin::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.extras"));
    engine->addImageProvider(QLatin1String("appbackgrounds"), new AppBackgroundProvider);
}

void PlasmaExtraComponentsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.extras"));
    qmlRegisterType<ResourceInstance>(uri, 0, 1, "ResourceInstance");
    qmlRegisterType<FallbackComponent>(uri, 0, 1, "FallbackComponent");
}


#include "plasmaextracomponentsplugin.moc"

