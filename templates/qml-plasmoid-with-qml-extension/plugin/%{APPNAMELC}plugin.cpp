/*
 *   Copyright (C) %{CURRENT_YEAR} by %{AUTHOR} <%{EMAIL}>                      *
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

#include "%{APPNAMELC}plugin.h"

#include <QJSEngine>
#include <QQmlEngine>
#include <QQmlContext>

static QJSValue singletonTypeExampleProvider(QQmlEngine* engine, QJSEngine* scriptEngine)
{
    Q_UNUSED(engine)

    QJSValue helloWorld = scriptEngine->newObject();
    helloWorld.setProperty("text", QStringLiteral("Hello world!"));
    return helloWorld;
}


void %{APPNAME}Plugin::registerTypes(const char* uri)
{
    Q_ASSERT(uri == QLatin1String("org.kde.plasma.private.%{APPNAMELC}"));

    qmlRegisterSingletonType(uri, 1, 0, "HelloWorld", singletonTypeExampleProvider);
}
