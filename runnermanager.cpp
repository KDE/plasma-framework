/*
 *   Copyright (C) 2006-2007 Ryan P. Bitanga <ryan.bitanga@gmail.com> 
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org> 
 *   Copyright 2008 Jordi Polo <mumismo@gmail.com>
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

#include <KServiceTypeTrader>
#include <KPluginInfo>
#include <KDebug>

#include <Solid/Device>
#include <Solid/DeviceInterface>

#include <threadweaver/DebuggingAids.h>
#include <ThreadWeaver/Thread>
#include <ThreadWeaver/Job>
#include <ThreadWeaver/State>
#include <ThreadWeaver/QueuePolicy>
#include <ThreadWeaver/Weaver>
#include <ThreadWeaver/WeaverObserver>
#include <QMutex>
#include <QTimer>
#include <QCoreApplication>

#include "querymatch.h"

#include "runnermanager.h"

using ThreadWeaver::Weaver;
using ThreadWeaver::Job;


namespace Plasma
{


/*****************************************************
*  RunnerRestrictionPolicy class
* Restricts simultaneous jobs of the same type
* Similar to ResourceRestrictionPolicy but check the object type first
******************************************************/
class RunnerRestrictionPolicy : public ThreadWeaver::QueuePolicy
{
public:
    ~RunnerRestrictionPolicy();

    static RunnerRestrictionPolicy& instance();

    void setCap(int cap)
    {
        m_cap = cap;
    }
    int cap() const
    {
        return m_cap;
    }

    bool canRun(Job* job);
    void free(Job* job);
    void release(Job* job);
    void destructed(Job* job);
private:
    RunnerRestrictionPolicy();

//     QHash<QString, int> m_runCounts;
    int m_count;
    int m_cap;
    QMutex m_mutex;
};

RunnerRestrictionPolicy::RunnerRestrictionPolicy()
    : QueuePolicy(),
      m_cap(2)
{
}

RunnerRestrictionPolicy::~RunnerRestrictionPolicy()
{
}

RunnerRestrictionPolicy& RunnerRestrictionPolicy::instance()
{
    static RunnerRestrictionPolicy policy;
    return policy;
}

bool RunnerRestrictionPolicy::canRun(Job* job)
{
    Q_UNUSED(job)
    QMutexLocker l(&m_mutex);
//     QString type = job->objectName();
    if (m_count/*m_runCounts.value(type)*/ > m_cap) {
//         kDebug() << "Denying job " << type << " because of " << m_count/*m_runCounts[type]*/ << " current jobs" << endl;
        return false;
    } else {
//         m_runCounts[type]++;
        ++m_count;
        return true;
    }
}

void RunnerRestrictionPolicy::free(Job* job)
{
    Q_UNUSED(job)
    QMutexLocker l(&m_mutex);
//     QString type = job->objectName();
    --m_count;
//     if (m_runCounts.value(type)) {
//         m_runCounts[type]--;
//     }
}

void RunnerRestrictionPolicy::release(Job* job)
{
    free(job);
}

void RunnerRestrictionPolicy::destructed(Job* job)
{
    Q_UNUSED(job)
}


/*****************************************************
*  FindMatchesJob class
* Class to run queries in different threads
******************************************************/
class FindMatchesJob : public Job
{
public:
    FindMatchesJob(const QString& term, Plasma::AbstractRunner* runner, Plasma::RunnerContext* context, QObject* parent = 0);

    int priority() const;

protected:
    void run();
private:
    QString m_term;
    Plasma::RunnerContext* m_context;
    Plasma::AbstractRunner* m_runner;
};

FindMatchesJob::FindMatchesJob( const QString& term, Plasma::AbstractRunner* runner, 
                                Plasma::RunnerContext* context, QObject* parent )
    : ThreadWeaver::Job(parent),
      m_term(term),
      m_context(context),
      m_runner(runner)
{
    if (runner->speed() == Plasma::AbstractRunner::SlowSpeed) {
        assignQueuePolicy(&RunnerRestrictionPolicy::instance());
    }
}

void FindMatchesJob::run()
{
//     kDebug() << "Running match for " << m_runner->objectName() << " in Thread " << thread()->id() << endl;
    m_runner->performMatch(*m_context);
}

int FindMatchesJob::priority() const
{
    return m_runner->priority();
}


/*****************************************************
*  RunnerManager::Private class
*
*****************************************************/
class RunnerManager::Private
{
public:

    Private(RunnerManager *parent)
      : q(parent)
    {
        matchChangeTimer.setSingleShot(true);
        connect(&matchChangeTimer, SIGNAL(timeout()), q, SLOT(scheduleMatchesChanged()));
        connect(&context, SIGNAL(matchesChanged()), q, SLOT(matchesChanged()));
    }

    void scheduleMatchesChanged()
    {
        matchChangeTimer.start(100);
    }

    void matchesChanged()
    {
        emit q->matchesChanged(context.matches());
    }

    void loadConfiguration(KConfigGroup& conf)
    {
        config = conf;

        //The number of threads used scales with the number of processors.     
        const int numProcs = qMax(Solid::Device::listFromType(Solid::DeviceInterface::Processor).count(), 1);
        //This entry allows to define a hard upper limit independent of the number of processors.
        const int maxThreads = config.readEntry("maxThreads",16);
        const int numThreads = qMin(maxThreads, 2 + ((numProcs - 1) * 2));
//        kDebug() << "setting up" << numThreads << "threads for" << numProcs << "processors";
        Weaver::instance()->setMaximumNumberOfThreads(numThreads);

        //Preferred order of execution of runners
        //prioritylist = config.readEntry("priority", QStringList());

        //If set, this list defines which runners won't be used at runtime
        blacklist = config.readEntry("blacklist", QStringList());
    }

    void loadAll()
    {
        AbstractRunner::List firstRunners;
        AbstractRunner::List normalRunners;
        AbstractRunner::List lastRunners;

        KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
        QString error;
        foreach (const KService::Ptr &service, offers) {
            //kDebug() << "Loading runner: " << service->name() << service->storageId();
            KPluginInfo description(service);
            QString runnerName = description.pluginName();
            if (blacklist.contains(runnerName)) {
                kDebug() << "The plugin" << service->name() << "was blackListed and will not load";
                continue;
            }
            QString api = service->property("X-Plasma-API").toString();
            AbstractRunner* runner = 0;

            if (api.isEmpty()) {
                QVariantList args;
                args << service->storageId();
                runner = service->createInstance<AbstractRunner>(q, args, &error);
            } else {
                //kDebug() << "got a script runner known as" << api;
                runner = new AbstractRunner(q, service->storageId());
            }

            if (runner) {
                //kDebug() << "loading runner: " << service->name();
                QString phase = service->property("X-Plasma-RunnerPhase").toString();
                if (phase == "last") {
                    lastRunners.append(runner);
                } else if (phase == "first") {
                    firstRunners.append(runner);
                } else {
                    normalRunners.append(runner);
                }
            } else {
                kDebug() << "failed to load runner : " << service->name() << ". error reported: " << error;
            }
        }

        firstRunners << normalRunners << lastRunners;
        runners.clear();
        runners = firstRunners;
        //kDebug() << "All runners loaded, total:" << runners.count();
    }

    RunnerManager *q;
    RunnerContext context;
    QTimer matchChangeTimer;
    AbstractRunner::List runners;
    QList<ThreadWeaver::Job*> searchJobs;
    QStringList prioritylist;
    QStringList blacklist;
    KConfigGroup config;
};



/*****************************************************
*  RunnerManager::Public class
*
*****************************************************/
RunnerManager::RunnerManager(QObject *parent)
    : QObject(parent), 
      d(new Private(this))
{
    KConfigGroup config(KGlobal::config(), "PlasmaRunnerManager");
    d->loadConfiguration(config);
    d->loadAll();
    //ThreadWeaver::setDebugLevel(true, 4);
       
}


RunnerManager::RunnerManager(KConfigGroup& config, QObject *parent)
    : QObject(parent), 
      d(new Private(this))
{
    d->loadConfiguration(config);
    d->loadAll();
    //ThreadWeaver::setDebugLevel(true, 4);
}

RunnerManager::~RunnerManager()
{
}

AbstractRunner* RunnerManager::runner(const QString &name) const
{
   //TODO: using a list of runners, if this gets slow, switch to hash
    foreach (Plasma::AbstractRunner* runner, d->runners) {
        //kDebug() << "comparing" << name << "with" << runner->id() << runner->name();
        if (name == runner->id()) {
            return runner;
        }
    }
    return 0;
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

void RunnerManager::run(const QueryMatch &match)
{
    //TODO: this function is not const as it may be used for learning
    match.run(d->context);
}

void RunnerManager::launchQuery (const QString & term, const QString & runnerName)
{
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
        runable.append(runner(runnerName));
    } else {
        runable = d->runners; 
    }

    bool jobsLaunched=false;
    foreach (Plasma::AbstractRunner* r, runable) {
        if ((r->ignoredTypes() & d->context.type()) == 0) {
//            kDebug() << "launching" << r->name();
            jobsLaunched=true;
            Job *job = new FindMatchesJob(term, r, &d->context, this);
            Weaver::instance()->enqueue( job );
            d->searchJobs.append( job );
        }
    }
}

bool RunnerManager::execQuery(const QString &term, const QString &runnerName)
{
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

void RunnerManager::reset()
{
    // If ThreadWeaver is idle, it is safe to clear previous jobs 
    if (Weaver::instance()->isIdle()) {
        qDeleteAll(d->searchJobs);
        d->searchJobs.clear();
    } else {
        Weaver::instance()->dequeue();
    }
    d->context.reset();

   //FIXME: if the weaver is not idle we have a number of jobs that 
   //      will eventually return their matches
   //      we need a way to stop them (Weaver::requestAbort() Job::requestAbort ) 
   //      or discard results 
}


}

#include "runnermanager.moc"
