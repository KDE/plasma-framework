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
#include "datasource.h"
#include "dialog.h"
#include "framesvgitem.h"
#include "iconitem.h"
#include "quicktheme.h"
#include "serviceoperationstatus.h"
#include "svgitem.h"
#include "theme.h"

#include "tooltip.h"
#include "units.h"
#include "windowthumbnail.h"
#include <plasma/servicejob.h>

// #include "dataenginebindings_p.h"

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

    qmlRegisterType<Plasma::DataSource>(uri, 2, 0, "DataSource");
    qmlRegisterType<Plasma::DataModel>(uri, 2, 0, "DataModel");
    qmlRegisterType<Plasma::SortFilterModel, 0>(uri, 2, 0, "SortFilterModel");
    qmlRegisterType<Plasma::SortFilterModel, 1>(uri, 2, 1, "SortFilterModel");

    qmlRegisterType<PlasmaQuick::Dialog>(uri, 2, 0, "Dialog");
    // HACK make properties like "opacity" work that are in REVISION 1 of QWindow
    qmlRegisterRevision<QWindow, 1>(uri, 2, 0);
    qmlRegisterRevision<QQuickItem, 1>(uri, 2, 0);
    qmlRegisterType<ToolTip>(uri, 2, 0, "ToolTipArea");

    // TODO KF6: check if it makes sense to call qmlRegisterInterface for any of these
    // as they seem currently not used as properties and are only used from JavaScript engine
    // due to being return types of Q_INVOKABLE methods,
    // so registering the pointers to the qobject meta-object system would be enough:
    // Plasma::Service, Plasma::ServiceJob
    // So this here would become just
    // qRegisterMetaType<Plasma::Service *>();
    // qRegisterMetaType<Plasma::ServiceJob *>();
    // For that also change all usages with those methods to use the fully namespaced type name
    // in the method signature.
    QT_WARNING_PUSH
    QT_WARNING_DISABLE_CLANG("-Wdeprecated-declarations")
    QT_WARNING_DISABLE_GCC("-Wdeprecated-declarations")
    // Do not to port these two for KF5 to
    // qmlRegisterInterface<Plasma::T>(uri, 1);
    // as this will incompatibly register with the fully namespaced name "Plasma::T",
    // not just the now explicitly passed alias name "T"
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    qmlRegisterInterface<Plasma::Service>("Service");
    qmlRegisterInterface<Plasma::ServiceJob>("ServiceJob");
#else
    qRegisterMetaType<Plasma::Service>();
    qRegisterMetaType<Plasma::ServiceJob>();
#endif
    QT_WARNING_POP

    qmlRegisterType<ServiceOperationStatus>(uri, 2, 0, "ServiceOperationStatus");
    qmlRegisterAnonymousType<QAbstractItemModel>(uri, 1);

    qmlRegisterAnonymousType<QQmlPropertyMap>(uri, 1);
    qmlRegisterType<IconItem>(uri, 2, 0, "IconItem");

    qmlRegisterType<Plasma::WindowThumbnail>(uri, 2, 0, "WindowThumbnail");
}
