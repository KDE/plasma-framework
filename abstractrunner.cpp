/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
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

#include "abstractrunner.h"

#include <QMutex>
#include <QMutexLocker>

#include <KDebug>
#include <KPluginInfo>
#include <KServiceTypeTrader>
#include <QTimer>

#include "scripting/runnerscript.h"
#include "searchcontext.h"

namespace Plasma
{

class AbstractRunner::Private
{
    public:
        bool hasMatchOptions;
        bool hasConfig;
        Priority priority;
        Speed speed;
        RunnerScript* script;
        KPluginInfo runnerDescription;
        AbstractRunner* runner;

    Private(AbstractRunner* r, KService::Ptr service)
      : priority(NormalPriority),
        speed(NormalSpeed),
        script(0),
        runnerDescription(service),
        runner(r)
    {
        if (runnerDescription.isValid()) {
            QString language = runnerDescription.property("X-Plasma-Language").toString();

            if (!language.isEmpty()) {
                script = Plasma::loadScriptEngine(language, runner);
                if (!script) {
                    kDebug() << "Could not create a" << language << "ScriptEngine for the"
                    << runnerDescription.name() << "Runner.";
                } else {
                    QTimer::singleShot(0, runner, SLOT(init()));
                }
            }
        }
    }

    static QMutex bigLock;
};

QMutex AbstractRunner::Private::bigLock;

AbstractRunner::AbstractRunner(QObject* parent, const QString& serviceId)
    : QObject(parent),
      d(new Private(this, KService::serviceByStorageId(serviceId)))
{
}

AbstractRunner::AbstractRunner(QObject* parent, const QVariantList& args)
    : QObject(parent),
      d(new Private(this, KService::serviceByStorageId(args.count() > 0 ? args[0].toString() : QString())))
{
}

AbstractRunner::~AbstractRunner()
{
    delete d;
}

KConfigGroup AbstractRunner::config() const
{
    QString group = objectName();
    if (group.isEmpty()) {
        group = "UnnamedRunner";
    }

    KConfigGroup runners(KGlobal::config(), "Runners");
    return KConfigGroup(&runners, group);
}

void AbstractRunner::performMatch( Plasma::SearchContext &globalContext )
{
    Plasma::SearchContext localContext( 0, globalContext );
    //Keep track of global context list sizes so we know which pointers are our responsibility to delete
    int exactEnd = localContext.exactMatches().count();
    int possibleEnd = localContext.possibleMatches().count();
    int infoEnd = localContext.informationalMatches().count();

    match( &localContext );

    QList<SearchMatch *> exact = localContext.exactMatches().mid(exactEnd);
    QList<SearchMatch *> possible = localContext.possibleMatches().mid(possibleEnd);
    QList<SearchMatch *> info = localContext.informationalMatches().mid(infoEnd);

    //If matches were not added, delete items on the heap
    if (!globalContext.addMatches(localContext.searchTerm(), exact, possible, info)) {
        qDeleteAll(exact);
        qDeleteAll(possible);
        qDeleteAll(info);
    }
}

bool AbstractRunner::hasMatchOptions()
{
    return d->hasMatchOptions;
}

void AbstractRunner::setHasMatchOptions(bool hasMatchOptions)
{
    d->hasMatchOptions = hasMatchOptions;
}

void AbstractRunner::createMatchOptions(QWidget *parent)
{
    Q_UNUSED(parent)
}

bool AbstractRunner::isConfigurable()
{
    return d->hasConfig;
}

void AbstractRunner::setIsConfigurable(bool hasConfig)
{
    d->hasConfig = hasConfig;
}

void AbstractRunner::createConfigurationInterface(QWidget *widget)
{
    Q_UNUSED(widget)
}

AbstractRunner::Speed AbstractRunner::speed() const
{
    return d->speed;
}

void AbstractRunner::setSpeed(Speed speed)
{
    d->speed = speed;
}

AbstractRunner::Priority AbstractRunner::priority() const
{
    return d->priority;
}

void AbstractRunner::setPriority(Priority priority)
{
    d->priority = priority;
}

KService::List AbstractRunner::serviceQuery(const QString &serviceType, const QString &constraint) const
{
    QMutexLocker lock(&Private::bigLock);
    return KServiceTypeTrader::self()->query(serviceType, constraint);
}

const QMutex& AbstractRunner::bigLock() const
{
    return Private::bigLock;
}

void AbstractRunner::exec(Plasma::SearchMatch *action)
{
    if (d->script) {
        return d->script->exec(action);
    }
}

void AbstractRunner::match(Plasma::SearchContext *search)
{
    if (d->script) {
        return d->script->match(search);
    }
}

QString AbstractRunner::runnerName() const
{
    if (!d->runnerDescription.isValid()) {
        return QString();
    }
    return d->runnerDescription.property("X-Plasma-RunnerName").toString();
}

void AbstractRunner::init()
{
    if (d->script) {
        d->script->init();
    }
}

AbstractRunner::List AbstractRunner::loadRunners(QObject* parent, const QStringList& whitelist)
{
    List firstRunners;
    List runners;
    List lastRunners;

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    QString error;
    QVariantList allArgs;
    foreach (KService::Ptr service, offers) {
        if( whitelist.empty() || whitelist.contains( service->name() ) ) {
            allArgs << service->storageId();
            QString language = service->property("X-Plasma-Language").toString();
            AbstractRunner* runner;
            if (language.isEmpty()) {
                runner = service->createInstance<AbstractRunner>(parent, allArgs, &error);
            } else {
                runner = new AbstractRunner(parent, service->storageId());
            }
            if (runner) {
                //kDebug() << "loaded runner : " << service->name();
                QString phase = service->property("X-Plasma-RunnerPhase").toString();
                if (phase == "last") {
                    lastRunners.append(runner);
                } else if (phase == "first") {
                    firstRunners.append(runner);
                } else {
                    runners.append(runner);
                }
            }
            else {
                kDebug() << "failed to load runner : " << service->name() << ". error reported: " << error;
            }
        }
    }

    firstRunners << runners << lastRunners;
    return firstRunners;
}

} // Plasma namespace

#include "abstractrunner.moc"
