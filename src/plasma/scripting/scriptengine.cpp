/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *
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

#include "scripting/scriptengine.h"

#include <QDebug>
#include <kservice.h>

#include "applet.h"
#include "dataengine.h"
#include "package.h"
#include "private/componentinstaller_p.h"
#include "scripting/appletscript.h"
#include "scripting/dataenginescript.h"

namespace Plasma
{

ScriptEngine::ScriptEngine(QObject *parent)
    : QObject(parent),
      d(0)
{
}

ScriptEngine::~ScriptEngine()
{
//    delete d;
}

bool ScriptEngine::init()
{
    return true;
}

Package ScriptEngine::package() const
{
    return Package();
}

QString ScriptEngine::mainScript() const
{
    return QString();
}

QStringList knownLanguages(Types::ComponentTypes types)
{
    QStringList languages;
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins("plasma/scriptengines");

    for (auto plugin : plugins) {
        if ((types & Types::AppletComponent) &&
            plugin.value("X-Plasma-ComponentTypes") == "Applet") {
            languages << plugin.value("X-Plasma-API");
        } else if ((types & Types::DataEngineComponent) &&
            plugin.value("X-Plasma-ComponentTypes") == "DataEngine") {
            languages << plugin.value("X-Plasma-API");
        }
    }

    return languages;
}

ScriptEngine *loadEngine(const QString &language, Types::ComponentType type, QObject *parent,
    const QVariantList &args = QVariantList())
{
    ScriptEngine *engine = 0;

    auto filter = [&language](const KPluginMetaData &md) -> bool
    {
        return md.value("X-Plasma-API") == language;
    };
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins("plasma/scriptengines", filter);

    if (plugins.count()) {
        if ((type & Types::AppletComponent) &&
            plugins.first().value("X-Plasma-ComponentTypes") != "Applet") {
            return 0;
        } else if ((type & Types::DataEngineComponent) &&
            plugins.first().value("X-Plasma-ComponentTypes") != "DataEngine") {
            return 0;
        }
        KPluginInfo::List lst = KPluginInfo::fromMetaData(plugins);
        KPluginLoader loader(lst.first().libraryPath());
        KPluginFactory *factory = loader.factory();
        if (factory) {
            engine = factory->create<Plasma::ScriptEngine>(0, args);
        }
    }

    return engine;
}

AppletScript *loadScriptEngine(const QString &language, Applet *applet, const QVariantList &args)
{
    AppletScript *engine =
        static_cast<AppletScript *>(loadEngine(language, Types::AppletComponent, applet, args));

    if (engine) {
        engine->setApplet(applet);
    }

    return engine;
}

DataEngineScript *loadScriptEngine(const QString &language, DataEngine *dataEngine, const QVariantList &args)
{
    DataEngineScript *engine =
        static_cast<DataEngineScript *>(loadEngine(language, Types::DataEngineComponent, dataEngine, args));

    if (engine) {
        engine->setDataEngine(dataEngine);
    }

    return engine;
}

} // namespace Plasma

#include "moc_scriptengine.cpp"
