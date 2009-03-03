/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007, 2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
 *   Copyright (C) 2008 Jordi Polo <mumismo@gmail.com>
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

#include "runnermanager.h"

#include <QMutex>
#include <QTimer>
#include <QCoreApplication>

#include <kdebug.h>
#include <kplugininfo.h>
#include <kservicetypetrader.h>
#include <kstandarddirs.h>

#include <solid/device.h>
#include <solid/deviceinterface.h>

#include <Weaver/DebuggingAids.h>
#include <Weaver/Thread.h>
#include <Weaver/ThreadWeaver.h>

#include "private/runnerjobs.h"
#include "querymatch.h"

using ThreadWeaver::Weaver;
using ThreadWeaver::Job;

namespace Plasma
{

/*****************************************************
*  RunnerManager::Private class
*
*****************************************************/
class RunnerManagerPrivate
{
public:

    RunnerManagerPrivate(RunnerManager *parent)
      : q(parent),
        deferredRun(0)
    {
        matchChangeTimer.setSingleShot(true);
        delayTimer.setSingleShot(true);

        QObject::connect(&matchChangeTimer, SIGNAL(timeout()), q, SLOT(matchesChanged()));
        QObject::connect(&context, SIGNAL(matchesChanged()), q, SLOT(scheduleMatchesChanged()));
        QObject::connect(&delayTimer, SIGNAL(timeout()), q, SLOT(unblockJobs()));
    }

    void scheduleMatchesChanged()
    {
        matchChangeTimer.start(0);
    }

    void matchesChanged()
    {
        emit q->matchesChanged(context.matches());
    }

    void loadConfiguration(KConfigGroup &conf)
    {
        config = conf;

        //The number of threads used scales with the number of processors.
        const int numProcs =
            qMax(Solid::Device::listFromType(Solid::DeviceInterface::Processor).count(), 1);
        //This entry allows to define a hard upper limit independent of the number of processors.
        const int maxThreads = config.readEntry("maxThreads", 16);
        const int numThreads = qMin(maxThreads, 2 + ((numProcs - 1) * 2));
        //kDebug() << "setting up" << numThreads << "threads for" << numProcs << "processors";
        Weaver::instance()->setMaximumNumberOfThreads(numThreads);
        // Limit the number of instances of a single normal speed runner and all of the slow runners
        // to half the number of threads
        const int cap = qMax(2, numThreads/2);
        DefaultRunnerPolicy::instance().setCap(cap);

        //If set, this list defines which runners won't be used at runtime
        //blacklist = config.readEntry("blacklist", QStringList());
    }

    void loadRunners()
    {
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");

        bool loadAll = config.readEntry("loadAll", false);
        //The plugin configuration is stored under the section Plugins
        //and not PlasmaRunnerManager->Plugins
        KConfigGroup conf(KGlobal::config(), "Plugins");

        foreach (const KService::Ptr &service, offers) {
            //kDebug() << "Loading runner: " << service->name() << service->storageId();
            QString tryExec = service->property("TryExec", QVariant::String).toString();
            kDebug() << "tryExec is" << tryExec;
            if (!tryExec.isEmpty() && KStandardDirs::findExe(tryExec).isEmpty()) {
                // we don't actually have this application!
                continue;
            }

            KPluginInfo description(service);
            QString runnerName = description.pluginName();
            description.load(conf);

            bool loaded = runners.contains(runnerName);
            bool selected = loadAll || description.isPluginEnabled();

            if (selected) {
                if (!loaded) {
                    QString api = service->property("X-Plasma-API").toString();
                    QString error;
                    AbstractRunner *runner = 0;

                    if (api.isEmpty()) {
                        QVariantList args;
                        args << service->storageId();
                        if (Plasma::isPluginVersionCompatible(KPluginLoader(*service).pluginVersion())) {
                            runner = service->createInstance<AbstractRunner>(q, args, &error);
                        }
                    } else {
                        //kDebug() << "got a script runner known as" << api;
                        runner = new AbstractRunner(q, service->storageId());
                    }

                    if (runner) {
                        kDebug() << "loading runner:" << service->name();
                        runners.insert(runnerName, runner);
                    } else {
                        kDebug() << "failed to load runner:" << service->name()
                                 << ". error reported:" << error;
                    }
                }
            } else if (loaded) {
                //Remove runner
                AbstractRunner *runner = runners.take(runnerName);
                kDebug() << "Removing runner: " << runnerName;
                delete runner;
            }
        }

        kDebug() << "All runners loaded, total:" << runners.count();
    }

    void jobDone(ThreadWeaver::Job *job)
    {
        FindMatchesJob *runJob = static_cast<FindMatchesJob*>(job);
        if (deferredRun.isEnabled() && runJob->runner() == deferredRun.runner()) {
            //kDebug() << "job actually done, running now **************";
            QueryMatch tmpRun = deferredRun;
            deferredRun = QueryMatch(0);	  
            tmpRun.run(context);
        }
        searchJobs.removeAll(runJob);
        delete runJob;
    }

    void unblockJobs()
    {
        // WORKAROUND: Queue an empty job to force ThreadWeaver to awaken threads
        // kDebug() << "- Unblocking jobs -" << endl;
        DummyJob *dummy = new DummyJob(q);
        Weaver::instance()->enqueue(dummy);
        QObject::connect(dummy, SIGNAL(done(ThreadWeaver::Job*)), dummy, SLOT(deleteLater()));
    }

    // Delay in ms before slow runners are allowed to run
    static const int slowRunDelay = 400;

    RunnerManager *q;
    QueryMatch deferredRun;
    RunnerContext context;
    QTimer matchChangeTimer;
    QTimer delayTimer; // Timer to control when to run slow runners
    QHash<QString, AbstractRunner*> runners;
    QList<FindMatchesJob*> searchJobs;
    bool loadAll;
    KConfigGroup config;
};

/*****************************************************
*  RunnerManager::Public class
*
*****************************************************/
RunnerManager::RunnerManager(QObject *parent)
    : QObject(parent),
      d(new RunnerManagerPrivate(this))
{
    KConfigGroup config(KGlobal::config(), "PlasmaRunnerManager");
    d->loadConfiguration(config);
    //ThreadWeaver::setDebugLevel(true, 4);
}

RunnerManager::RunnerManager(KConfigGroup &c, QObject *parent)
    : QObject(parent),
      d(new RunnerManagerPrivate(this))
{
    // Should this be really needed? Maybe d->loadConfiguration(c) would make
    // more sense.
    KConfigGroup config(&c, "PlasmaRunnerManager");
    d->loadConfiguration(config);
    //ThreadWeaver::setDebugLevel(true, 4);
}

RunnerManager::~RunnerManager()
{
    delete d;
}

void RunnerManager::reloadConfiguration()
{
    d->loadConfiguration(d->config);
    d->loadRunners();
}

AbstractRunner* RunnerManager::runner(const QString &name) const
{
    if (d->runners.isEmpty()) {
        d->loadRunners();
    }

    return d->runners.value(name, 0);
}

RunnerContext* RunnerManager::searchContext() const
{
    return &d->context;
}

//Reordering is here so data is not reordered till strictly needed
QList<QueryMatch> RunnerManager::matches() const
{
    return d->context.matches();
}

void RunnerManager::run(const QString &id)
{
    run(d->context.match(id));
}

void RunnerManager::run(const QueryMatch &match)
{
    if (!match.isEnabled()) {
        return;
    }

    //TODO: this function is not const as it may be used for learning
    AbstractRunner *runner = match.runner();

    foreach (FindMatchesJob *job, d->searchJobs) {
        if (job->runner() == runner && !job->isFinished()) {
            //kDebug() << "!!!!!!!!!!!!!!!!!!! uh oh!";
            d->deferredRun = match;
            return;
        }
    }

    if (d->deferredRun.isValid()) {
        d->deferredRun = QueryMatch(0);

    match.run(d->context);


    }
}

QList<QAction*> RunnerManager::actionsForMatch(const QueryMatch &match)
{
    AbstractRunner *runner = match.runner();
    if (runner) {
        return runner->actionsForMatch(match);
    }

    return QList<QAction*>();
}

void RunnerManager::launchQuery(const QString &term)
{
    launchQuery(term, QString());
}

void RunnerManager::launchQuery(const QString &term, const QString &runnerName)
{
    if (d->runners.isEmpty()) {
        d->loadRunners();
    }

    if (term.isEmpty()) {
        reset();
        return;
    }

    if (d->context.query() == term) {
        // we already are searching for this!
        return;
    }

    reset();
//    kDebug() << "runners searching for" << term << "on" << runnerName;
    d->context.setQuery(term);

    AbstractRunner::List runable;

    //if the name is not empty we will launch only the specified runner
    if (!runnerName.isEmpty()) {
        AbstractRunner *r = runner(runnerName);
        if (r) {
            runable.append(r);
        }
    } else {
        runable = d->runners.values();
    }

    foreach (Plasma::AbstractRunner *r, runable) {
        if ((r->ignoredTypes() & d->context.type()) == 0) {
//            kDebug() << "launching" << r->name();
            FindMatchesJob *job = new FindMatchesJob(r, &d->context, this);
            connect(job, SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(jobDone(ThreadWeaver::Job*)));
            if (r->speed() == AbstractRunner::SlowSpeed) {
                job->setDelayTimer(&d->delayTimer);
            }
            Weaver::instance()->enqueue(job);
            d->searchJobs.append(job);
        }
    }
    // Start timer to unblock slow runners
    d->delayTimer.start(RunnerManagerPrivate::slowRunDelay);
}

bool RunnerManager::execQuery(const QString &term)
{
    return execQuery(term, QString());
}

bool RunnerManager::execQuery(const QString &term, const QString &runnerName)
{
    if (d->runners.isEmpty()) {
        d->loadRunners();
    }

    if (term.isEmpty()) {
        reset();
        return false;
    }

    if (d->context.query() == term) {
        // we already are searching for this!
        emit matchesChanged(d->context.matches());
        return false;
    }

    reset();
    //kDebug() << "executing query about " << term << "on" << runnerName;
    d->context.setQuery(term);
    AbstractRunner *r = runner(runnerName);

    if (!r) {
        //kDebug() << "failed to find the runner";
        return false;
    }

    if ((r->ignoredTypes() & d->context.type()) != 0) {
        //kDebug() << "ignored!";
        return false;
    }

    r->performMatch(d->context);
    //kDebug() << "succeeded with" << d->context.matches().count() << "results";
    emit matchesChanged(d->context.matches());
    return true;
}

QString RunnerManager::query() const
{
    return d->context.query();
}

void RunnerManager::reset()
{
    // If ThreadWeaver is idle, it is safe to clear previous jobs
    if (Weaver::instance()->isIdle()) {
        qDeleteAll(d->searchJobs);
        d->searchJobs.clear();
    } else {
        Weaver::instance()->dequeue();
    }

    if (d->deferredRun.isEnabled()) {
        //kDebug() << "job actually done, running now **************";
        QueryMatch tmpRun = d->deferredRun;
        d->deferredRun = QueryMatch(0);
        tmpRun.run(d->context);
    }

    d->context.reset();
}

} // Plasma namespace

#include "runnermanager.moc"
