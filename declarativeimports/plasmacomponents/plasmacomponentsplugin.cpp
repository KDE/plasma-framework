/*
 *   Copyright 2011 by Marco Martin <mart@kde.org>

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

#include "plasmacomponentsplugin.h"

#include <QtDeclarative/qdeclarative.h>
#include <QtDeclarative/QDeclarativeEngine>
#include <QtDeclarative/QDeclarativeContext>
#include <QtDeclarative/QDeclarativeItem>

#include "qrangemodel.h"

#include <KSharedConfig>
#include <KDebug>
#include <KGlobal>

#include <kdeclarative.h>

#include "enums.h"
#include "qmenu.h"
#include "qmenuitem.h"
#include "fullscreensheet.h"

Q_EXPORT_PLUGIN2(plasmacomponentsplugin, PlasmaComponentsPlugin)

class BKSingleton
{
public:
   EngineBookKeeping self;
};
K_GLOBAL_STATIC(BKSingleton, privateBKSelf)

EngineBookKeeping::EngineBookKeeping()
{
}

EngineBookKeeping *EngineBookKeeping::self()
{
    return &privateBKSelf->self;
}

QDeclarativeEngine *EngineBookKeeping::engine() const
{
    //for components creation, any engine will do, as long is valid
    if (m_engines.isEmpty()) {
        kWarning() << "No engines found, this should never happen";
        return 0;
    } else {
        return m_engines.values().first();
    }
}

void EngineBookKeeping::insertEngine(QDeclarativeEngine *engine)
{
    connect(engine, SIGNAL(destroyed(QObject *)),
            this, SLOT(engineDestroyed(QObject *)));
    m_engines.insert(engine);
}

void EngineBookKeeping::engineDestroyed(QObject *deleted)
{
    m_engines.remove(static_cast<QDeclarativeEngine *>(deleted));
}



void PlasmaComponentsPlugin::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    QDeclarativeExtensionPlugin::initializeEngine(engine, uri);
    EngineBookKeeping::self()->insertEngine(engine);
}

void PlasmaComponentsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.components"));

    //platform specific c++ components
    const QString target = KDeclarative::componentsTarget();
    if (target == KDeclarative::defaultComponentsTarget()) {
        qmlRegisterType<QMenuProxy>(uri, 0, 1, "Menu");
        qmlRegisterType<QMenuItem>(uri, 0, 1, "MenuItem");
    } else {
        qmlRegisterType<FullScreenSheet>(uri, 0, 1, "Sheet");
    }

    qmlRegisterType<Plasma::QRangeModel>(uri, 0, 1, "RangeModel");

    qmlRegisterUncreatableType<DialogStatus>(uri, 0, 1, "DialogStatus", "");
    qmlRegisterUncreatableType<PageOrientation>(uri, 0, 1, "PageOrientation", "");
    qmlRegisterUncreatableType<PageStatus>(uri, 0, 1, "PageStatus", "");
}


#include "plasmacomponentsplugin.moc"

