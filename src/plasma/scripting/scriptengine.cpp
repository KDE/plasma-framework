/*
    SPDX-FileCopyrightText: 2007 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "scripting/scriptengine.h"

#include <QDebug>
#include <QGlobalStatic>

#include "applet.h"
#include "dataengine.h"
#include "debug_p.h"
#include "package.h"
#include "scripting/appletscript.h"
#include "scripting/dataenginescript.h"

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
        if (((types & Types::AppletComponent) && componentTypes.contains(QLatin1String("Applet")))
            || ((types & Types::DataEngineComponent) && componentTypes.contains(QLatin1String("DataEngine")))) {
            ret << plugin;
        }
    }
    return ret;
}

ScriptEngine::ScriptEngine(QObject *parent)
    : QObject(parent)
    , d(nullptr)
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

#if PLASMA_BUILD_DEPRECATED_SINCE(5, 83)
Package ScriptEngine::package() const
{
    return Package();
}
#endif

QString ScriptEngine::mainScript() const
{
    return QString();
}

QStringList knownLanguages(Types::ComponentTypes types)
{
    QStringList languages;
    const QVector<KPluginMetaData> plugins = listEngines(types, [](const KPluginMetaData &) -> bool {
        return true;
    });

    for (const auto &plugin : plugins)
        languages << plugin.value(QStringLiteral("X-Plasma-API"));

    return languages;
}

typedef QHash<QString, QSharedPointer<KPluginLoader>> EngineCache;
Q_GLOBAL_STATIC(EngineCache, engines)

ScriptEngine *loadEngine(const QString &language, Types::ComponentType type, QObject *parent, const QVariantList &args = QVariantList())
{
    Q_UNUSED(parent);

    {
        auto it = engines->constFind(language);
        if (it != engines->constEnd()) {
            return (*it)->factory()->create<Plasma::ScriptEngine>(nullptr, args);
        }
    }

    ScriptEngine *engine = nullptr;
    auto filter = [&language](const KPluginMetaData &md) -> bool {
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
    AppletScript *engine = static_cast<AppletScript *>(loadEngine(language, Types::AppletComponent, applet, args));

    if (engine) {
        engine->setApplet(applet);
    }

    return engine;
}

DataEngineScript *loadScriptEngine(const QString &language, DataEngine *dataEngine, const QVariantList &args)
{
    DataEngineScript *engine = static_cast<DataEngineScript *>(loadEngine(language, Types::DataEngineComponent, dataEngine, args));

    if (engine) {
        engine->setDataEngine(dataEngine);
    }

    return engine;
}

} // namespace Plasma

#include "moc_scriptengine.cpp"
