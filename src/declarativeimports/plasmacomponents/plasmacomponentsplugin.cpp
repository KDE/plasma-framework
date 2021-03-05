/*
    SPDX-FileCopyrightText: 2011 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasmacomponentsplugin.h"

#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>

#include "qrangemodel.h"

#include <KSharedConfig>
#include <QDebug>

#include <kdeclarative/kdeclarative.h>

#include "enums.h"
#include "qmenu.h"
#include "qmenuitem.h"

class BKSingleton
{
public:
    EngineBookKeeping self;
};
Q_GLOBAL_STATIC(BKSingleton, privateBKSelf)

EngineBookKeeping::EngineBookKeeping()
{
}

EngineBookKeeping *EngineBookKeeping::self()
{
    return &privateBKSelf->self;
}

QQmlEngine *EngineBookKeeping::engine() const
{
    // for components creation, any engine will do, as long is valid
    if (m_engines.isEmpty()) {
        qWarning() << "No engines found, this should never happen";
        return nullptr;
    } else {
        return *m_engines.constBegin();
    }
}

void EngineBookKeeping::insertEngine(QQmlEngine *engine)
{
    connect(engine, &QObject::destroyed, this, &EngineBookKeeping::engineDestroyed);
    m_engines.insert(engine);
}

void EngineBookKeeping::engineDestroyed(QObject *deleted)
{
    m_engines.remove(static_cast<QQmlEngine *>(deleted));
}

void PlasmaComponentsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);
    EngineBookKeeping::self()->insertEngine(engine);
}

void PlasmaComponentsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.plasma.components"));

    qmlRegisterType<QMenuProxy>(uri, 2, 0, "Menu");
    qmlRegisterType<QMenuItem>(uri, 2, 0, "MenuItem");

    qmlRegisterType<Plasma::QRangeModel>(uri, 2, 0, "RangeModel");

    qmlRegisterUncreatableType<DialogStatus>(uri, 2, 0, "DialogStatus", {});
    qmlRegisterUncreatableType<PageOrientation>(uri, 2, 0, "PageOrientation", {});
    qmlRegisterUncreatableType<PageStatus>(uri, 2, 0, "PageStatus", {});
}

#include "moc_plasmacomponentsplugin.cpp"
