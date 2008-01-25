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

#include <KDebug>
#include <KService>
#include <KServiceTypeTrader>

#include "abstractrunner.h"
#include "applet.h"
#include "dataengine.h"
#include "package.h"
#include "scripting/appletscript.h"
#include "scripting/dataenginescript.h"
#include "scripting/runnerscript.h"

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

QStringList knownLanguages(ComponentTypes types)
{
    QString constraintTemplate = "'%1' in [X-Plasma-ComponentTypes]";
    QString constraint;

    if (types & AppletComponent) {
        // currently this if statement is not needed, but this future proofs
        // the code against someone initializing constraint to something
        // before we get here.
        if (!constraint.isEmpty()) {
            constraint.append(" or ");
        }

        constraint.append(constraintTemplate.arg("Applet"));
    }

    if (types & DataEngineComponent) {
        if (!constraint.isEmpty()) {
            constraint.append(" or ");
        }

        constraint.append(constraintTemplate.arg("DataEngine"));
    }

    if (types & RunnerComponent) {
        if (!constraint.isEmpty()) {
            constraint.append(" or ");
        }

        constraint.append(constraintTemplate.arg("Runner"));
    }

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine", constraint);
    //kDebug() << "Applet::knownApplets constraint was '" << constraint << "' which got us " << offers.count() << " matches";

    QStringList languages;
    foreach (KService::Ptr service, offers) {
        QString language = service->property("X-Plasma-Language").toString();
        if (!languages.contains(language)) {
            languages.append(language);
        }
    }

    return languages;
}

ScriptEngine* loadEngine(const QString &language, ComponentType type, QObject *parent)
{
    if (language.isEmpty()) {
        return 0;
    }

    QRegExp re("[^a-zA-Z0-9\\-_]");
    if (re.indexIn(language) != -1) {
        kDebug() << "invalid language attempted:" << language;
        return 0;
    }

    QString component;
    switch (type) {
        case AppletComponent:
            component = "Applet";
            break;
        case DataEngineComponent:
            component = "DataEngine";
            break;
        case RunnerComponent:
            component = "Runner";
            break;
        default:
            return 0;
            break;
    }

    QString constraint = QString("[X-Plasma-Language] == '%1' and "
                                 "'%2' in [X-Plasma-ComponentTypes]").arg(language, component);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine", constraint);
/*    kDebug() << "********************* loadingApplet with Plasma/ScriptEngine" << constraint
             << "resulting in" << offers.count() << "results";*/
    if (offers.isEmpty()) {
        kDebug() << "ScriptEngine::load: no offers for \"" << language << "\"";
        return 0;
    }

    QVariantList args;
    QString error;

    ScriptEngine *engine = 0;
    foreach (KService::Ptr service, offers) {
        switch (type) {
            case AppletComponent:
                engine = service->createInstance<Plasma::AppletScript>(parent, args, &error);
                break;
            case DataEngineComponent:
                engine = service->createInstance<Plasma::DataEngineScript>(parent, args, &error);
                break;
            case RunnerComponent:
                engine = service->createInstance<Plasma::RunnerScript>(parent, args, &error);
                break;
            default:
                return 0;
                break;
        }

        if (engine) {
            return engine;
        }

        kDebug() << "Couldn't load script engine for language " << language << "! error reported: " << error;
    }

    return 0;
}

AppletScript* loadScriptEngine(const QString &language, Applet *applet)
{
    AppletScript *engine = static_cast<AppletScript*>(loadEngine(language, AppletComponent, applet));

    if (engine) {
        engine->setApplet(applet);
    }

    return engine;
}

DataEngineScript* loadScriptEngine(const QString &language, DataEngine *dataEngine)
{
    return static_cast<DataEngineScript*>(loadEngine(language, DataEngineComponent, dataEngine));
}

RunnerScript* loadScriptEngine(const QString &language, AbstractRunner *runner)
{
    return static_cast<RunnerScript*>(loadEngine(language, RunnerComponent, runner));
}

} // namespace Plasma

#include <scriptengine.moc>

