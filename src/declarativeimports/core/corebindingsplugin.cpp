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
#include <QScriptEngine>

#include <kdeclarative.h>

#include <plasma/framesvg.h>
#include <plasma/svg.h>
#include <Plasma/QueryMatch>
#include <Plasma/ServiceJob>

#include "datasource.h"
#include "datamodel.h"
#include "framesvgitem.h"
#include "runnermodel.h"
#include "svgitem.h"
#include "theme.h"
#include "dialog.h"
#include "iconitem.h"
#include "tooltipwindow.h"

#include "tooltip.h"

// #include "dataenginebindings_p.h"
#include "plasmanamespace.h"

#include <QDebug>

void CoreBindingsPlugin::initializeEngine(QQmlEngine *engine, const char *uri)
{
    QQmlExtensionPlugin::initializeEngine(engine, uri);

    QQmlContext *context = engine->rootContext();

    ThemeProxy *theme = new ThemeProxy(context);
    context->setContextProperty("theme", theme);

/*    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine);
    kdeclarative.initialize();
    QScriptEngine *scriptEngine = kdeclarative.scriptEngine();

    //inject the hack only if wasn't injected already
    if (!scriptEngine->globalObject().property("i18n").isValid()) {
        //binds things like kconfig and icons
        kdeclarative.setupBindings();
    }

    registerDataEngineMetaTypes(scriptEngine);
    */
    //qDebug() << "====> org.kde.plasma.core loaded.";
}

void CoreBindingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.core"));

    qmlRegisterUncreatableType<PlasmaNamespace>(uri, 2, 0, "Plasma", "");

    qmlRegisterType<Plasma::Svg>(uri, 2, 0, "Svg");
    qmlRegisterType<Plasma::FrameSvg>(uri, 2, 0, "FrameSvg");
    qmlRegisterType<Plasma::SvgItem>(uri, 2, 0, "SvgItem");
    qmlRegisterType<Plasma::FrameSvgItem>(uri, 2, 0, "FrameSvgItem");
 
    qmlRegisterType<ThemeProxy>(uri, 2, 0, "Theme");

    qmlRegisterType<Plasma::DataSource>(uri, 2, 0, "DataSource");
    qmlRegisterType<Plasma::DataModel>(uri, 2, 0, "DataModel");
    qmlRegisterType<Plasma::SortFilterModel>(uri, 2, 0, "SortFilterModel");

    qmlRegisterType<DialogProxy>(uri, 2, 0, "Dialog");
    qmlRegisterType<ToolTipWindow>(uri, 2, 0, "ToolTipWindow");
    qmlRegisterType<ToolTip>(uri, 2, 0, "ToolTipProxy");

    qmlRegisterInterface<Plasma::Service>("Service");
    qRegisterMetaType<Plasma::Service*>("Service");
    qmlRegisterInterface<Plasma::ServiceJob>("ServiceJob");
    qRegisterMetaType<Plasma::ServiceJob*>("ServiceJob");
    qmlRegisterType<QAbstractItemModel>();

    qmlRegisterType<RunnerModel>(uri, 2, 0, "RunnerModel");
    qmlRegisterInterface<Plasma::QueryMatch>("QueryMatch");
    qRegisterMetaType<Plasma::QueryMatch *>("QueryMatch");

    qmlRegisterType<QQmlPropertyMap>();
    qmlRegisterType<IconItem>(uri, 2, 0, "IconItem");

    qmlRegisterInterface<Plasma::DataSource>("DataSource");
    qRegisterMetaType<Plasma::DataSource*>("DataSource");

}


#include "corebindingsplugin.moc"

