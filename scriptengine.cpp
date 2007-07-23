/*
 *   Copyright 2007 by Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include "scriptengine.h"

#include <KDebug>
#include <KService>
#include <KServiceTypeTrader>

#include "applet.h"
#include "package.h"

namespace Plasma
{

class ScriptEngine::Private
{
public:
    Applet* applet;
};

ScriptEngine::ScriptEngine(QObject *parent, const QStringList &args)
    : QObject(parent),
      d(new Private)
{
    Q_UNUSED(args)
    d->applet = 0;
}

ScriptEngine::~ScriptEngine()
{
}

void ScriptEngine::init(Applet* applet)
{
    d->applet = applet;
    if (!init()) {
        d->applet->setFailedToLaunch(true);
    }
}

bool ScriptEngine::init()
{
    Q_ASSERT(d->applet);
    return false;
}

QString ScriptEngine::mainScript() const
{
    Q_ASSERT(d->applet);
    return d->applet->package()->filePath("mainscript");
}

Applet* ScriptEngine::applet()  const
{
    Q_ASSERT(d->applet);
    return d->applet;
}

const Package* ScriptEngine::package() const
{
    Q_ASSERT(d->applet);
    return d->applet->package();
}

QStringList ScriptEngine::knownLanguages()
{
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine");
    //kDebug() << "Applet::knownApplets constraint was '" << constraint << "' which got us " << offers.count() << " matches" << endl;

    QStringList languages;
    foreach (KService::Ptr service, offers) {
        QString language = service->property("X-Plasma-Language").toString();
        if (!languages.contains(language)) {
            languages.append(language);
        }
    }

    return languages;
}

ScriptEngine* ScriptEngine::load(const QString &language, Applet *applet)
{
    if (language.isEmpty()) {
        return 0;
    }

    QString constraint = QString("[X-Plasma-Language] == '%1'").arg(language);
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine", constraint);

    if (offers.isEmpty()) {
        kDebug() << "ScriptEngine::load: no offers for \"" << language << "\"" << endl;
        return 0;
    }

    QStringList args;
    ScriptEngine* engine = 0;
    foreach (KService::Ptr service, offers) {
        engine = KService::createInstance<Plasma::ScriptEngine>(service, applet, args);
        if (engine) {
            break;
        }
    }

    if (!engine) {
        kDebug() << "Couldn't load script engine for language " << language << "!" << endl;
        return 0;
    }

    engine->init(applet);
    return engine;
}

} // namespace Plasma

#include <scriptengine.moc>

