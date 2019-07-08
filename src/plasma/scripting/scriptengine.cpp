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
#include <QGlobalStatic>

#include "applet.h"
#include "dataengine.h"
#include "package.h"
#include "scripting/appletscript.h"
#include "scripting/dataenginescript.h"
#include "debug_p.h"

namespace Plasma
{

static QVector<KPluginMetaData> listEngines(Types::ComponentTypes types, std::function<bool(const KPluginMetaData &)> filter)
{
    QVector<KPluginMetaData> ret;
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("plasma/scriptengines"));
    ret.reserve(plugins.size());
    for (const auto &plugin : plugins) {
        if (!filter(plugin))
            continue;
        const QStringList componentTypes = KPluginMetaData::readStringList(plugins.first().rawData(), QStringLiteral("X-Plasma-ComponentTypes"));
        if (((types & Types::AppletComponent)     && componentTypes.contains(QStringLiteral("Applet")))
          ||((types & Types::DataEngineComponent) && componentTypes.contains(QStringLiteral("DataEngine")))) {
            ret << plugin;
        }
    }
    return ret;
}

ScriptEngine::ScriptEngine(QObject *parent)
    : QObject(parent),
      d(nullptr)
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
    const QVector<KPluginMetaData> plugins = listEngines(types, [] (const KPluginMetaData &) -> bool { return true;});

    for (const auto &plugin : plugins)
        languages << plugin.value(QStringLiteral("X-Plasma-API"));

    return languages;
}

typedef QHash<QString, QSharedPointer<KPluginLoader>> EngineCache;
Q_GLOBAL_STATIC(EngineCache, engines)

ScriptEngine *loadEngine(const QString &language, Types::ComponentType type, QObject *parent,
    const QVariantList &args = QVariantList())
{
    Q_UNUSED(parent);

    {
        auto it = engines->constFind(language);
        if (it != engines->constEnd()) {
            return (*it)->factory()->create<Plasma::ScriptEngine>(nullptr, args);
        }
    }

    ScriptEngine *engine = nullptr;
    auto filter = [&language](const KPluginMetaData &md) -> bool
    {
        return md.value(QStringLiteral("X-Plasma-API")) == language;
    };

    const QVector<KPluginMetaData> plugins = listEngines(type, filter);
    if (!plugins.isEmpty()) {
        QSharedPointer<KPluginLoader> loader(new KPluginLoader(plugins.first().fileName()));
        KPluginFactory *factory = loader->factory();
        if (factory) {
            engine = factory->create<Plasma::ScriptEngine>(nullptr, args);
            engines->insert(language, loader);
        } else {
            qCWarning(LOG_PLASMA) << "Unable to load" << plugins.first().name() << "ScriptEngine";
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
