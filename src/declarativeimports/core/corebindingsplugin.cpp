/*
    SPDX-FileCopyrightText: 2009 Alan Alpert <alan.alpert@nokia.com>
    SPDX-FileCopyrightText: 2010 Ménard Alexis <menard@kde.org>
    SPDX-FileCopyrightText: 2010 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "corebindingsplugin.h"

#include <QQmlContext>

#include <KLocalizedContext>

#include <plasma/framesvg.h>
#include <plasma/svg.h>

#include "colorscope.h"
#include "datamodel.h"
#include "dialog.h"
#include "framesvgitem.h"
#include "iconitem.h"
#include "quicktheme.h"
#include "svgitem.h"
#include "theme.h"

#include "tooltip.h"
#include "units.h"
#include "windowthumbnail.h"

#include <QDebug>
#include <QWindow>

void CoreBindingsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);

    QQmlContext *context = engine->rootContext();

    Plasma::QuickTheme *theme = new Plasma::QuickTheme(engine);

    if (!qEnvironmentVariableIntValue("PLASMA_NO_CONTEXTPROPERTIES")) {
        context->setContextProperty(QStringLiteral("theme"), theme);
        context->setContextProperty(QStringLiteral("units"), &Units::instance());
    }

    if (!context->contextObject()) {
        KLocalizedContext *localizedContextObject = new KLocalizedContext(engine);
        context->setContextObject(localizedContextObject);
    }
}

void CoreBindingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QByteArray("org.kde.plasma.core"));

    qmlRegisterUncreatableType<Plasma::Types>(uri, 2, 0, "Types", {});
    qmlRegisterSingletonType<Units>(uri, 2, 0, "Units", [](QQmlEngine *engine, QJSEngine *) -> QObject * {
        engine->setObjectOwnership(&Units::instance(), QQmlEngine::CppOwnership);
        return &Units::instance();
    });

    qmlRegisterType<Plasma::Svg>(uri, 2, 0, "Svg");
    qmlRegisterType<Plasma::FrameSvg>(uri, 2, 0, "FrameSvg");
    qmlRegisterType<Plasma::SvgItem>(uri, 2, 0, "SvgItem");
    qmlRegisterType<Plasma::FrameSvgItem>(uri, 2, 0, "FrameSvgItem");

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
    qmlRegisterType<IconItem>(uri, 2, 0, "IconItem");

    qmlRegisterType<Plasma::WindowThumbnail>(uri, 2, 0, "WindowThumbnail");
}

#include "moc_corebindingsplugin.cpp"
