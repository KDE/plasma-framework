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
#include "debug_p.h"

namespace Plasma
{

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
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("plasma/scriptengines"));

    foreach (const auto &plugin, plugins) {
        const QStringList componentTypes = KPluginMetaData::readStringList(plugins.first().rawData(), QStringLiteral("X-Plasma-ComponentTypes"));
        if (((types & Types::AppletComponent)     && componentTypes.contains(QStringLiteral("Applet")))
          ||((types & Types::DataEngineComponent) && componentTypes.contains(QStringLiteral("DataEngine")))) {
            languages << plugin.value(QStringLiteral("X-Plasma-API"));
        }
    }

    return languages;
}

ScriptEngine *loadEngine(const QString &language, Types::ComponentType type, QObject *parent,
    const QVariantList &args = QVariantList())
{
    Q_UNUSED(parent);

    ScriptEngine *engine = nullptr;

    auto filter = [&language](const KPluginMetaData &md) -> bool
    {
        return md.value(QStringLiteral("X-Plasma-API")) == language;
    };
    QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("plasma/scriptengines"), filter);

    if (!plugins.isEmpty()) {
        const QStringList componentTypes = KPluginMetaData::readStringList(plugins.first().rawData(), QStringLiteral("X-Plasma-ComponentTypes"));
        if (((type & Types::AppletComponent)     && !componentTypes.contains(QStringLiteral("Applet")))
         || ((type & Types::DataEngineComponent) && !componentTypes.contains(QStringLiteral("DataEngine")))) {

            qCWarning(LOG_PLASMA) << "ScriptEngine" << plugins.first().name() << "does not provide Applet or DataEngine components, returning empty.";
            return nullptr;
        }
        KPluginLoader loader(plugins.first().fileName());
        KPluginFactory *factory = loader.factory();
        if (factory) {
            engine = factory->create<Plasma::ScriptEngine>(nullptr, args);
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
