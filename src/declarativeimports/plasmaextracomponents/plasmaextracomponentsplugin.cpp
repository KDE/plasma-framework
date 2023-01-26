/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "plasmaextracomponentsplugin.h"

#include "appbackgroundprovider_p.h"
#include "fallbackcomponent.h"

#include <QQmlEngine>
#include <QtQml>

// #include <KSharedConfig>
// #include <KConfigGroup>

#include "qmenu.h"
#include "qmenuitem.h"

void PlasmaExtraComponentsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.plasma.extras"));
    engine->addImageProvider(QStringLiteral("appbackgrounds"), new AppBackgroundProvider);
}

void PlasmaExtraComponentsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.plasma.extras"));
    qmlRegisterType<FallbackComponent>(uri, 2, 0, "FallbackComponent");
    qmlRegisterType<QMenuProxy>(uri, 2, 0, "Menu");
    qmlRegisterType<QMenuItem>(uri, 2, 0, "MenuItem");
    qmlRegisterUncreatableType<DialogStatus>(uri, 2, 0, "DialogStatus", {});
}
