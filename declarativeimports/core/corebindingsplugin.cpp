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

#include <Plasma/FrameSvg>
#include <Plasma/Svg>

#include "datasource_p.h"
#include "datamodel.h"
#include "framesvgitem_p.h"
#include "svgitem_p.h"
#include "theme_p.h"
#include "dialog.h"
#include "tooltip.h"

void CoreBindingsPlugin::initializeEngine(QDeclarativeEngine *engine, const char *uri)
{
    QDeclarativeExtensionPlugin::initializeEngine(engine, uri);

    QDeclarativeContext *context = engine->rootContext();

    ThemeProxy *theme = new ThemeProxy(context);
    context->setContextProperty("theme", theme);
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

    /*qmlRegisterInterface<Plasma::DataSource>("DataSource");
    qRegisterMetaType<Plasma::DataSource*>("DataSource");*/
}


#include "corebindingsplugin.moc"

