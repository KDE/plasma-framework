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

ScriptEngine::ScriptEngine(QObject *parent)
    : QObject(parent),
      d(new Private)
{
    d->applet = 0;
}

ScriptEngine::~ScriptEngine()
{
    delete d;
}

void ScriptEngine::init(Applet* applet)
{
    d->applet = applet;
    if (!init()) {
        d->applet->setFailedToLaunch(true);
    }
}

void ScriptEngine::paintInterface(QPainter* painter, const QStyleOptionGraphicsItem* option, const QRect &contentsRect)
{
    Q_UNUSED(painter)
    Q_UNUSED(option)
    Q_UNUSED(contentsRect)
}

QSizeF ScriptEngine::size() const
{
    if (applet()) {
        return applet()->contentSize();
    }

    return QSizeF();
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

DataEngine* ScriptEngine::dataEngine(const QString &engine) const
{
    Q_ASSERT(d->applet);
    return d->applet->dataEngine(engine);
}

const Package* ScriptEngine::package() const
{
    Q_ASSERT(d->applet);
    return d->applet->package();
}

QStringList ScriptEngine::knownLanguages()
{
    KService::List offers = KServiceTypeTrader::self()->query("Plasma/ScriptEngine");
    //kDebug(1209) << "Applet::knownApplets constraint was '" << constraint << "' which got us " << offers.count() << " matches";

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
        kDebug(1209) << "ScriptEngine::load: no offers for \"" << language << "\"";
        return 0;
    }

    QVariantList args;
    QString error;
    ScriptEngine* engine = 0;
    foreach (KService::Ptr service, offers) {
        engine = service->createInstance<Plasma::ScriptEngine>(applet, args, &error);

        if (engine) {
            break;
        }

        kDebug(1209) << "Couldn't load script engine for language " << language << "! error reported: " << error;
    }

    if (!engine) {
        return 0;
    }

    engine->init(applet);
    return engine;
}

} // namespace Plasma

#include <scriptengine.moc>

