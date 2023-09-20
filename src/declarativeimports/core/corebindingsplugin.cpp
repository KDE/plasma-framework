/*
    SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
    SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "corebindingsplugin.h"

#include <QActionGroup>
#include <QQmlContext>

#include <KLocalizedContext>

#include "action.h"
#include "dialog.h"
#include "theme.h"

#include "tooltip.h"
#include "windowthumbnail.h"

#include <QDebug>
#include <QQmlPropertyMap>
#include <QWindow>
#include <qqml.h>

void CoreBindingsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);

    QQmlContext *context = engine->rootContext();

    if (!context->contextObject()) {
        KLocalizedContext *localizedContextObject = new KLocalizedContext(engine);
        context->setContextObject(localizedContextObject);
    }
    // This ensures that importing plasmacore will make any KSvg use the current lasma theme
    new Plasma::Theme(engine);
}

void CoreBindingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.plasma.core"));

    qmlRegisterUncreatableType<Plasma::Types>(uri, 2, 0, "Types", {});

    qmlRegisterSingletonType<Plasma::Theme>(uri, 2, 0, "Theme", [](QQmlEngine *engine, QJSEngine *) -> QObject * {
        return new Plasma::Theme(engine);
    });

    qmlRegisterType<PlasmaQuick::Dialog>(uri, 2, 0, "Dialog");
    // HACK make properties like "opacity" work that are in REVISION 1 of QWindow
    qmlRegisterRevision<QWindow, 1>(uri, 2, 0);
    qmlRegisterRevision<QQuickItem, 1>(uri, 2, 0);
    qmlRegisterType<ToolTip>(uri, 2, 0, "ToolTipArea");

    qmlRegisterAnonymousType<QQmlPropertyMap>(uri, 1);

    qmlRegisterType<Plasma::WindowThumbnail>(uri, 2, 0, "WindowThumbnail");

    qmlRegisterAnonymousType<IconGroup>(uri, 2);
    qmlRegisterExtendedType<QAction, ActionExtension>(uri, 2, 0, "Action");
    qmlRegisterType<ActionGroup>(uri, 2, 0, "ActionGroup");
}

#include "moc_corebindingsplugin.cpp"
