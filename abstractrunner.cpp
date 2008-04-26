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
#include <KStandardDirs>
#include <QTimer>

#include "package.h"
#include "scripting/runnerscript.h"
#include "searchcontext.h"

namespace Plasma
{

class AbstractRunner::Private
{
public:
    Private(AbstractRunner* r, KService::Ptr service)
      : priority(NormalPriority),
        speed(NormalSpeed),
        script(0),
        runnerDescription(service),
        runner(r),
        fastRuns(0),
        package(0)
    {
        if (runnerDescription.isValid()) {
            const QString language = runnerDescription.property("X-Plasma-Language").toString();
            if (!language.isEmpty()) {
                const QString path = KStandardDirs::locate("data",
                                    "plasma/runners/" + runnerDescription.pluginName() + "/");
                PackageStructure::Ptr structure = Plasma::packageStructure(language, Plasma::RunnerComponent);
                structure->setPath(path);
                package = new Package(path, structure);

                script = Plasma::loadScriptEngine(language, runner);
                if (!script) {
                    kDebug() << "Could not create a" << language << "ScriptEngine for the"
                    << runnerDescription.name() << "Runner.";
                    delete package;
                    package = 0;
                } else {
                    QTimer::singleShot(0, runner, SLOT(init()));
                }
            }
        }
    }

    bool hasMatchOptions;
    bool hasConfig;
    Priority priority;
    Speed speed;
    RunnerScript* script;
    KPluginInfo runnerDescription;
    AbstractRunner* runner;
    QTime runtime;
    int fastRuns;
    Package *package;

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
    static const int reasonableRunTime = 1500;
    static const int fastEnoughTime = 250;

    d->runtime.restart();
    SearchContext localContext(0, globalContext);

    match(&localContext);

    // automatically rate limit runners that become slooow
    const int runtime = d->runtime.elapsed();
    bool slowed = speed() == SlowSpeed;

    if (!slowed && runtime > reasonableRunTime) {
        // we punish runners that return too slowly, even if they don't bring
        // back matches
        kDebug() << name() << "runner is too slow, putting it on the back burner.";
        d->fastRuns = 0;
        setSpeed(SlowSpeed);
    }

    //If matches were not added, delete items on the heap
    if (localContext.moveMatchesTo(globalContext) &&
        slowed && runtime < fastEnoughTime) {
        ++d->fastRuns;

        if (d->fastRuns > 2) {
            // we reward slowed runners who bring back matches fast enough
            // 3 times in a row
            kDebug() << name() << "runner is faster than we thought, kicking it up a notch";
            setSpeed(NormalSpeed);
        }
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

QMutex* AbstractRunner::bigLock() const
{
    return &Private::bigLock;
}

void AbstractRunner::exec(const Plasma::SearchContext *search, const Plasma::SearchMatch *action)
{
    if (d->script) {
        return d->script->exec(search, action);
    }
}

void AbstractRunner::match(Plasma::SearchContext *search)
{
    if (d->script) {
        return d->script->match(search);
    }
}

QString AbstractRunner::name() const
{
    if (!d->runnerDescription.isValid()) {
        return objectName();
    }
    return d->runnerDescription.property("X-Plasma-RunnerName").toString();
}

const Package* AbstractRunner::package() const
{
    return d->package;
}

void AbstractRunner::init()
{
    if (d->script) {
        d->script->init();
    }
}

AbstractRunner::List AbstractRunner::load(QObject* parent, const QStringList& whitelist)
{
    List firstRunners;
    List runners;
    List lastRunners;

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    QString error;
    foreach (KService::Ptr service, offers) {
        if (whitelist.empty() || whitelist.contains(service->name())) {
            QString language = service->property("X-Plasma-Language").toString();
            AbstractRunner* runner = 0;

            if (language.isEmpty()) {
                QVariantList args;
                args << service->storageId();
                runner = service->createInstance<AbstractRunner>(parent, args, &error);
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
            } else {
                kDebug() << "failed to load runner : " << service->name() << ". error reported: " << error;
            }
        }
    }

    firstRunners << runners << lastRunners;
    return firstRunners;
}

} // Plasma namespace

#include "abstractrunner.moc"
