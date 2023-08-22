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
#include "colorscope.h"
#include "datamodel.h"
#include "dialog.h"
#include "quicktheme.h"

#include "tooltip.h"
#include "units.h"
#include "windowthumbnail.h"

#include <QDebug>
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
    qmlRegisterSingletonType<Units>(uri, 2, 0, "Units", [](QQmlEngine *engine, QJSEngine *) -> QObject * {
        engine->setObjectOwnership(&Units::instance(), QQmlEngine::CppOwnership);
        return &Units::instance();
    });

    // qmlRegisterType<ThemeProxy>(uri, 2, 0, "Theme");
    qmlRegisterSingletonType<Plasma::QuickTheme>(uri, 2, 0, "Theme", [](QQmlEngine *engine, QJSEngine *) -> QObject * {
        return new Plasma::QuickTheme(engine);
    });
    qmlRegisterType<ColorScope>(uri, 2, 0, "ColorScope");

    // TODO: remove
    qmlRegisterType<Plasma::SortFilterModel, 0>(uri, 2, 0, "SortFilterModel");
    qmlRegisterType<Plasma::SortFilterModel, 1>(uri, 2, 1, "SortFilterModel");

    qmlRegisterType<PlasmaQuick::Dialog>(uri, 2, 0, "Dialog");
    // HACK make properties like "opacity" work that are in REVISION 1 of QWindow
    qmlRegisterRevision<QWindow, 1>(uri, 2, 0);
    qmlRegisterRevision<QQuickItem, 1>(uri, 2, 0);
    qmlRegisterType<ToolTip>(uri, 2, 0, "ToolTipArea");

    qmlRegisterAnonymousType<QAbstractItemModel>(uri, 1);

    qmlRegisterAnonymousType<QQmlPropertyMap>(uri, 1);

    qmlRegisterType<Plasma::WindowThumbnail>(uri, 2, 0, "WindowThumbnail");

    qmlRegisterAnonymousType<IconGroup>(uri, 2);
    qmlRegisterExtendedType<QAction, ActionExtension>(uri, 2, 0, "Action");
    qmlRegisterType<ActionGroup>(uri, 2, 0, "ActionGroup");
}

#include "moc_corebindingsplugin.cpp"
