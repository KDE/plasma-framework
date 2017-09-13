/*
 *   Copyright 2009 by Alan Alpert <alan.alpert@nokia.com>
 *   Copyright 2010 by Ménard Alexis <menard@kde.org>
 *   Copyright 2010 by Marco Martin <mart@kde.org>
 *   Copyright 2013 by Sebastian Kügler <sebas@kde.org>

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

#include "corebindingsplugin.h"

#include <QQmlContext>

#include <kdeclarative/kdeclarative.h>

#include <plasma/framesvg.h>
#include <plasma/svg.h>

#include "datasource.h"
#include "datamodel.h"
#include "framesvgitem.h"
#include "svgitem.h"
#include "theme.h"
#include "dialog.h"
#include "iconitem.h"
#include "serviceoperationstatus.h"
#include "colorscope.h"
#include "quicktheme.h"

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
    context->setContextProperty(QStringLiteral("theme"), theme);

    context->setContextProperty(QStringLiteral("units"), &Units::instance());

    if (!engine->rootContext()->contextObject()) {
        KDeclarative::KDeclarative kdeclarative;
        kdeclarative.setDeclarativeEngine(engine);
        kdeclarative.setupBindings();
    }
}

void CoreBindingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.core"));

    qmlRegisterUncreatableType<Plasma::Types>(uri, 2, 0, "Types", "");
    qmlRegisterUncreatableType<Units>(uri, 2, 0, "Units", "");

    qmlRegisterType<Plasma::Svg>(uri, 2, 0, "Svg");
    qmlRegisterType<Plasma::FrameSvg>(uri, 2, 0, "FrameSvg");
    qmlRegisterType<Plasma::SvgItem>(uri, 2, 0, "SvgItem");
    qmlRegisterType<Plasma::FrameSvgItem>(uri, 2, 0, "FrameSvgItem");

    //qmlRegisterType<ThemeProxy>(uri, 2, 0, "Theme");
    qmlRegisterUncreatableType<Plasma::QuickTheme>(uri, 2, 0, "Theme", QStringLiteral("It is not possible to instantiate Theme directly."));
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

    qmlRegisterInterface<Plasma::Service>("Service");
    qRegisterMetaType<Plasma::Service *>("Service");
    qmlRegisterInterface<Plasma::ServiceJob>("ServiceJob");
    qRegisterMetaType<Plasma::ServiceJob *>("ServiceJob");
    qmlRegisterType<ServiceOperationStatus>(uri, 2, 0, "ServiceOperationStatus");
    qmlRegisterType<QAbstractItemModel>();

    qmlRegisterType<QQmlPropertyMap>();
    qmlRegisterType<IconItem>(uri, 2, 0, "IconItem");

    qmlRegisterInterface<Plasma::DataSource>("DataSource");
    qRegisterMetaType<Plasma::DataSource *>("DataSource");

    qmlRegisterType<Plasma::WindowThumbnail>(uri, 2, 0, "WindowThumbnail");
}


