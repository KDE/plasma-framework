/*
 *   Copyright 2009 by Alan Alpert <alan.alpert@nokia.com>
 *   Copyright 2010 by Ménard Alexis <menard@kde.org>
 *   Copyright 2010 by Marco Martin <mart@kde.org>

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

#include <QtDeclarative/qdeclarative.h>
#include <QDeclarativeContext>
#include <QScriptEngine>

#include <kdeclarative.h>

#include <Plasma/FrameSvg>
#include <Plasma/Svg>
#include <Plasma/QueryMatch>

#include "datasource.h"
#include "datamodel.h"
#include "framesvgitem.h"
#include "runnermodel.h"
#include "svgitem.h"
#include "theme.h"
#include "dialog.h"
#include "iconitem.h"
#include "tooltip.h"
#include "dataenginebindings_p.h"

void CoreBindingsPlugin::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    QDeclarativeExtensionPlugin::initializeEngine(engine, uri);

    QDeclarativeContext *context = engine->rootContext();

    ThemeProxy *theme = new ThemeProxy(context);
    context->setContextProperty("theme", theme);

    KDeclarative kdeclarative;
    kdeclarative.setDeclarativeEngine(engine);
    kdeclarative.initialize();
    QScriptEngine *scriptEngine = kdeclarative.scriptEngine();

    //inject the hack only if wasn't injected already
    if (!scriptEngine->globalObject().property("i18n").isValid()) {
        //binds things like kconfig and icons
        kdeclarative.setupBindings();
    }

    registerDataEngineMetaTypes(scriptEngine);
}

void CoreBindingsPlugin::registerTypes(const char *uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.core"));

    qmlRegisterType<Plasma::Svg>(uri, 0, 1, "Svg");
    qmlRegisterType<Plasma::FrameSvg>(uri, 0, 1, "FrameSvg");
    qmlRegisterType<Plasma::SvgItem>(uri, 0, 1, "SvgItem");
    qmlRegisterType<Plasma::FrameSvgItem>(uri, 0, 1, "FrameSvgItem");

    qmlRegisterType<ThemeProxy>(uri, 0, 1, "Theme");

    qmlRegisterType<Plasma::DataSource>(uri, 0, 1, "DataSource");
    qmlRegisterType<Plasma::DataModel>(uri, 0, 1, "DataModel");
    qmlRegisterType<Plasma::SortFilterModel>(uri, 0, 1, "SortFilterModel");

    qmlRegisterType<DialogProxy>(uri, 0, 1, "Dialog");
    qmlRegisterType<ToolTipProxy>(uri, 0, 1, "ToolTip");

    qmlRegisterInterface<Plasma::Service>("Service");
    qRegisterMetaType<Plasma::Service*>("Service");
    qmlRegisterInterface<Plasma::ServiceJob>("ServiceJob");
    qRegisterMetaType<Plasma::ServiceJob*>("ServiceJob");
    qmlRegisterType<QAbstractItemModel>();

    qmlRegisterType<RunnerModel>(uri, 0, 1, "RunnerModel");
    qmlRegisterInterface<Plasma::QueryMatch>("QueryMatch");
    qRegisterMetaType<Plasma::QueryMatch *>("QueryMatch");

    qmlRegisterType<QDeclarativePropertyMap>();
    qmlRegisterType<IconItem>(uri, 0, 1, "IconItem");

    /*qmlRegisterInterface<Plasma::DataSource>("DataSource");
    qRegisterMetaType<Plasma::DataSource*>("DataSource");*/
}


#include "corebindingsplugin.moc"

