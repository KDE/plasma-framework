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

#include "config-plasma.h"

#include <QMutex>
#include <QTimer>
#include <QCoreApplication>
#include <qstandardpaths.h>

#include <QDebug>
#include <kplugininfo.h>
#include <kservicetypetrader.h>

#if !PLASMA_NO_SOLID
#include <solid/device.h>
#include <solid/deviceinterface.h>
#endif

#include <ThreadWeaver/DebuggingAids>
#include <ThreadWeaver/Queue>
#include <ThreadWeaver/Thread>

#include "private/runnerjobs_p.h"
#include "pluginloader.h"
#include "querymatch.h"

using ThreadWeaver::Queue;
using ThreadWeaver::Job;

//#define MEASURE_PREPTIME

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
        deferredRun(0),
        currentSingleRunner(0),
        prepped(false),
        allRunnersPrepped(false),
        singleRunnerPrepped(false),
        teardownRequested(false),
        singleMode(false),
        singleRunnerWasLoaded(false)
    {
        matchChangeTimer.setSingleShot(true);
        delayTimer.setSingleShot(true);

        QObject::connect(&matchChangeTimer, SIGNAL(timeout()), q, SLOT(matchesChanged()));
        QObject::connect(&context, SIGNAL(matchesChanged()), q, SLOT(scheduleMatchesChanged()));
        QObject::connect(&delayTimer, SIGNAL(timeout()), q, SLOT(unblockJobs()));
    }

    ~RunnerManagerPrivate()
    {
        KConfigGroup config = configGroup();
        context.save(config);
    }

    void scheduleMatchesChanged()
    {
        matchChangeTimer.start(100);
    }

    void matchesChanged()
    {
        emit q->matchesChanged(context.matches());
    }

    void loadConfiguration()
    {
        KConfigGroup config = configGroup();

        //The number of threads used scales with the number of processors.
#if !PLASMA_NO_SOLID
        const int numProcs =
            qMax(Solid::Device::listFromType(Solid::DeviceInterface::Processor).count(), 1);
#else
        const int numProcs = 1;
#endif
        //This entry allows to define a hard upper limit independent of the number of processors.
        const int maxThreads = config.readEntry("maxThreads", 16);
        const int numThreads = qMin(maxThreads, 2 + ((numProcs - 1) * 2));
        //qDebug() << "setting up" << numThreads << "threads for" << numProcs << "processors";
        if (numThreads > Queue::instance()->maximumNumberOfThreads()) {
            Queue::instance()->setMaximumNumberOfThreads(numThreads);
        }
        // Limit the number of instances of a single normal speed runner and all of the slow runners
        // to half the number of threads
        const int cap = qMax(2, numThreads/2);
        DefaultRunnerPolicy::instance().setCap(cap);

        context.restore(config);
    }

    KConfigGroup configGroup()
    {
        return conf.isValid() ? conf : KConfigGroup(KSharedConfig::openConfig(), "PlasmaRunnerManager");
    }

    void clearSingleRunner()
    {
        if (singleRunnerWasLoaded) {
            delete currentSingleRunner;
        }

        currentSingleRunner = 0;
    }

    void loadSingleRunner()
    {
        if (!singleMode || singleModeRunnerId.isEmpty()) {
            clearSingleRunner();
            return;
        }

        if (currentSingleRunner) {
            if (currentSingleRunner->id() == singleModeRunnerId) {
                return;
            }

            clearSingleRunner();
        }

        AbstractRunner *loadedRunner = q->runner(singleModeRunnerId);
        if (loadedRunner) {
            singleRunnerWasLoaded = false;
            currentSingleRunner = loadedRunner;
            return;
        }

        KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner", QString("[X-KDE-PluginInfo-Name] == '%1'").arg(singleModeRunnerId));
        if (!offers.isEmpty()) {
            const KService::Ptr &service = offers[0];
            currentSingleRunner = loadInstalledRunner(service);

            if (currentSingleRunner) {
                emit currentSingleRunner->prepare();
                singleRunnerWasLoaded = true;
            }
        }
    }

    void loadRunners()
    {
        KConfigGroup config = configGroup();
        KPluginInfo::List offers = RunnerManager::listRunnerInfo();

        const bool loadAll = config.readEntry("loadAll", false);
        const QStringList whiteList = config.readEntry("pluginWhiteList", QStringList());
        const bool noWhiteList = whiteList.isEmpty();
        KConfigGroup pluginConf;
        if (conf.isValid()) {
            pluginConf = KConfigGroup(&conf, "Plugins");
        } else {
            pluginConf = KConfigGroup(KSharedConfig::openConfig(), "Plugins");
        }

        advertiseSingleRunnerIds.clear();

        QSet<AbstractRunner *> deadRunners;
        QMutableListIterator<KPluginInfo> it(offers);
        while (it.hasNext()) {
            KPluginInfo &description = it.next();
            //qDebug() << "Loading runner: " << service->name() << service->storageId();
            QString tryExec = description.property("TryExec").toString();
            //qDebug() << "TryExec is" << tryExec;
            if (!tryExec.isEmpty() && QStandardPaths::findExecutable(tryExec).isEmpty()) {
                // we don't actually have this application!
                continue;
            }

            const QString runnerName = description.pluginName();
            description.load(pluginConf);

            const bool loaded = runners.contains(runnerName);
            const bool selected = loadAll || (description.isPluginEnabled() && (noWhiteList || whiteList.contains(runnerName)));

            const bool singleQueryModeEnabled = description.property("X-Plasma-AdvertiseSingleRunnerQueryMode").toBool();

            if (singleQueryModeEnabled) {
                advertiseSingleRunnerIds.insert(runnerName, description.name());
            }

            //qDebug() << loadAll << description.isPluginEnabled() << noWhiteList << whiteList.contains(runnerName);
            if (selected) {
                if (!loaded) {
                    AbstractRunner *runner = loadInstalledRunner(description.service());

                    if (runner) {
                        runners.insert(runnerName, runner);
                    }
                }
            } else if (loaded) {
                //Remove runner
                deadRunners.insert(runners.take(runnerName));
#ifndef NDEBUG
                // qDebug() << "Removing runner: " << runnerName;
#endif
            }
        }

        if (!deadRunners.isEmpty()) {
                QSet<QSharedPointer<FindMatchesJob> > deadJobs;
                auto it = searchJobs.begin();
                while (it != searchJobs.end()) {
                    auto &job = (*it);
                    if (deadRunners.contains(job->runner())) {
                        QObject::disconnect(job->decorator(), SIGNAL(done(ThreadWeaver::JobPointer)), q, SLOT(jobDone(ThreadWeaver::JobPointer)));
                        it = searchJobs.erase(it);
                        deadJobs.insert(job);
                    } else {
                        it++;
                    }
                }

                it = oldSearchJobs.begin();
                while (it != oldSearchJobs.end()) {
                    auto &job = (*it);
                    if (deadRunners.contains(job->runner())) {
                        it = oldSearchJobs.erase(it);
                        deadJobs.insert(job);
                    } else {
                        it++;
                    }
                }

            if (deadJobs.isEmpty()) {
                qDeleteAll(deadRunners);
            } else {
                new DelayedJobCleaner(deadJobs, deadRunners);
            }
        }

        if (!singleRunnerWasLoaded) {
            // in case we deleted it up above
            clearSingleRunner();
        }

#ifndef NDEBUG
        // qDebug() << "All runners loaded, total:" << runners.count();
#endif
    }

    AbstractRunner *loadInstalledRunner(const KService::Ptr service)
    {
        if (!service) {
            return 0;
        }

        AbstractRunner *runner = PluginLoader::self()->loadRunner(service->property("X-KDE-PluginInfo-Name", QVariant::String).toString());

        if (runner) {
            runner->setParent(q);
        } else {
            const QString api = service->property("X-Plasma-API").toString();

            if (api.isEmpty()) {
                QVariantList args;
                args << service->storageId();
                if (Plasma::isPluginVersionCompatible(KPluginLoader(*service).pluginVersion())) {
                    QString error;
                    runner = service->createInstance<AbstractRunner>(q, args, &error);
                    if (!runner) {
#ifndef NDEBUG
                        // qDebug() << "Failed to load runner:" << service->name() << ". error reported:" << error;
#endif
                    }
                }
            } else {
                //qDebug() << "got a script runner known as" << api;
                runner = new AbstractRunner(service, q);
            }
        }

        if (runner) {
#ifndef NDEBUG
            // qDebug() << "================= loading runner:" << service->name() << "=================";
#endif
            QObject::connect(runner, SIGNAL(matchingSuspended(bool)), q, SLOT(runnerMatchingSuspended(bool)));
            runner->init();
            if (prepped) {
                emit runner->prepare();
            }
        }

        return runner;
    }

    void jobDone(ThreadWeaver::JobPointer job)
    {
        auto runJob = job.dynamicCast<FindMatchesJob>();

        if (!runJob) {
            return;
        }

        if (deferredRun.isEnabled() && runJob->runner() == deferredRun.runner()) {
            //qDebug() << "job actually done, running now **************";
            QueryMatch tmpRun = deferredRun;
            deferredRun = QueryMatch(0);
            tmpRun.run(context);
        }

        searchJobs.remove(runJob);
        oldSearchJobs.remove(runJob);

        if (searchJobs.isEmpty() && context.matches().isEmpty()) {
            // we finished our run, and there are no valid matches, and so no
            // signal will have been sent out. so we need to emit the signal
            // ourselves here
            emit q->matchesChanged(context.matches());
        }

        checkTearDown();
    }

    void checkTearDown()
    {
        //qDebug() << prepped << teardownRequested << searchJobs.count() << oldSearchJobs.count();

        if (!prepped || !teardownRequested) {
            return;
        }

        if (Queue::instance()->isIdle()) {
            searchJobs.clear();
            oldSearchJobs.clear();
        }

        if (searchJobs.isEmpty() && oldSearchJobs.isEmpty()) {
            if (allRunnersPrepped) {
                foreach (AbstractRunner *runner, runners) {
                    emit runner->teardown();
                }

                allRunnersPrepped = false;
            }

            if (singleRunnerPrepped) {
                if (currentSingleRunner) {
                    emit currentSingleRunner->teardown();
                }

                singleRunnerPrepped = false;
            }

            emit q->queryFinished();

            prepped = false;
            teardownRequested = false;
        }
    }

    void unblockJobs()
    {
        if (searchJobs.isEmpty() && Queue::instance()->isIdle()) {
            oldSearchJobs.clear();
            checkTearDown();
            return;
        }

        Queue::instance()->reschedule();
    }

    void runnerMatchingSuspended(bool suspended)
    {
        if (suspended || !prepped || teardownRequested) {
            return;
        }

        AbstractRunner *runner = qobject_cast<AbstractRunner *>(q->sender());

        if (runner) {
            startJob(runner);
        }
    }

    void startJob(AbstractRunner *runner)
    {
        if ((runner->ignoredTypes() & context.type()) == 0) {
            QSharedPointer<FindMatchesJob> job(new FindMatchesJob(runner, &context, Queue::instance()));
            QObject::connect(job->decorator(), SIGNAL(done(ThreadWeaver::JobPointer)), q, SLOT(jobDone(ThreadWeaver::JobPointer)));
            if (runner->speed() == AbstractRunner::SlowSpeed) {
                job->setDelayTimer(&delayTimer);
            }
            Queue::instance()->enqueue(job);
            searchJobs.insert(job);
        }
    }

    // Delay in ms before slow runners are allowed to run
    static const int slowRunDelay = 400;

    RunnerManager *q;
    QueryMatch deferredRun;
    RunnerContext context;
    QTimer matchChangeTimer;
    QTimer delayTimer; // Timer to control when to run slow runners
    QHash<QString, AbstractRunner*> runners;
    QHash<QString, QString> advertiseSingleRunnerIds;
    AbstractRunner* currentSingleRunner;
    QSet<QSharedPointer<FindMatchesJob> > searchJobs;
    QSet<QSharedPointer<FindMatchesJob> > oldSearchJobs;
    KConfigGroup conf;
    QString singleModeRunnerId;
    bool loadAll : 1;
    bool prepped : 1;
    bool allRunnersPrepped : 1;
    bool singleRunnerPrepped : 1;
    bool teardownRequested : 1;
    bool singleMode : 1;
    bool singleRunnerWasLoaded : 1;
};

/*****************************************************
*  RunnerManager::Public class
*
*****************************************************/
RunnerManager::RunnerManager(QObject *parent)
    : QObject(parent),
      d(new RunnerManagerPrivate(this))
{
    d->loadConfiguration();
    //ThreadWeaver::setDebugLevel(true, 4);
}

RunnerManager::RunnerManager(KConfigGroup &c, QObject *parent)
    : QObject(parent),
      d(new RunnerManagerPrivate(this))
{
    // Should this be really needed? Maybe d->loadConfiguration(c) would make
    // more sense.
    d->conf = KConfigGroup(&c, "PlasmaRunnerManager");
    d->loadConfiguration();
    //ThreadWeaver::setDebugLevel(true, 4);
}

RunnerManager::~RunnerManager()
{
    if (!qApp->closingDown() && (!d->searchJobs.isEmpty() || !d->oldSearchJobs.isEmpty())) {
        new DelayedJobCleaner(d->searchJobs + d->oldSearchJobs);
    }

    delete d;
}

void RunnerManager::reloadConfiguration()
{
    d->loadConfiguration();
    d->loadRunners();
}

void RunnerManager::setAllowedRunners(const QStringList &runners)
{
    KConfigGroup config = d->configGroup();
    config.writeEntry("pluginWhiteList", runners);

    if (!d->runners.isEmpty()) {
        // this has been called with runners already created. so let's do an instant reload
        d->loadRunners();
    }
}

QStringList RunnerManager::allowedRunners() const
{
    KConfigGroup config = d->configGroup();
    return config.readEntry("pluginWhiteList", QStringList());
}

void RunnerManager::loadRunner(const KService::Ptr service)
{
    KPluginInfo description(service);
    const QString runnerName = description.pluginName();
    if (!runnerName.isEmpty() && !d->runners.contains(runnerName)) {
        AbstractRunner *runner = d->loadInstalledRunner(service);
        if (runner) {
            d->runners.insert(runnerName, runner);
        }
    }
}

void RunnerManager::loadRunner(const QString &path)
{
    if (!d->runners.contains(path)) {
        AbstractRunner *runner = new AbstractRunner(this, path);
        connect(runner, SIGNAL(matchingSuspended(bool)), this, SLOT(runnerMatchingSuspended(bool)));
        d->runners.insert(path, runner);
    }
}

AbstractRunner* RunnerManager::runner(const QString &name) const
{
    if (d->runners.isEmpty()) {
        d->loadRunners();
    }

    return d->runners.value(name, 0);
}

AbstractRunner *RunnerManager::singleModeRunner() const
{
    return d->currentSingleRunner;
}

void RunnerManager::setSingleModeRunnerId(const QString &id)
{
    d->singleModeRunnerId = id;
    d->loadSingleRunner();
}

QString RunnerManager::singleModeRunnerId() const
{
    return d->singleModeRunnerId;
}

bool RunnerManager::singleMode() const
{
    return d->singleMode;
}

void RunnerManager::setSingleMode(bool singleMode)
{
    if (d->singleMode == singleMode) {
        return;
    }


    Plasma::AbstractRunner *prevSingleRunner = d->currentSingleRunner;
    d->singleMode = singleMode;
    d->loadSingleRunner();
    d->singleMode = d->currentSingleRunner;

    if (prevSingleRunner != d->currentSingleRunner) {
        if (d->prepped) {
            matchSessionComplete();

            if (d->singleMode) {
                setupMatchSession();
            }
        }
    }
}

QList<AbstractRunner *> RunnerManager::runners() const
{
    return d->runners.values();
}

QStringList RunnerManager::singleModeAdvertisedRunnerIds() const
{
    return d->advertiseSingleRunnerIds.keys();
}

QString RunnerManager::runnerName(const QString &id) const
{
    if (runner(id)) {
        return runner(id)->name();
    } else {
        return d->advertiseSingleRunnerIds.value(id, QString());
    }
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

void RunnerManager::run(const QString &matchId)
{
    run(d->context.match(matchId));
}

void RunnerManager::run(const QueryMatch &match)
{
    if (!match.isEnabled()) {
        return;
    }

    //TODO: this function is not const as it may be used for learning
    AbstractRunner *runner = match.runner();

    for (auto it = d->searchJobs.constBegin(); it != d->searchJobs.constEnd(); ++it) {
        if ((*it)->runner() == runner && !(*it)->isFinished()) {
#ifndef NDEBUG
            // qDebug() << "deferred run";
#endif
            d->deferredRun = match;
            return;
        }
    }

    if (d->deferredRun.isValid()) {
        d->deferredRun = QueryMatch(0);
    }

    d->context.run(match);
}

QList<QAction*> RunnerManager::actionsForMatch(const QueryMatch &match)
{
    AbstractRunner *runner = match.runner();
    if (runner) {
        return runner->actionsForMatch(match);
    }

    return QList<QAction*>();
}

QMimeData * RunnerManager::mimeDataForMatch(const QString &id) const
{
    return mimeDataForMatch(d->context.match(id));
}


QMimeData * RunnerManager::mimeDataForMatch(const QueryMatch &match) const
{
    AbstractRunner *runner = match.runner();
    if (runner) {
        return runner->mimeDataForMatch(match);
    }

    return 0;
}

KPluginInfo::List RunnerManager::listRunnerInfo(const QString &parentApp)
{
    return PluginLoader::self()->listRunnerInfo(parentApp);
}

void RunnerManager::setupMatchSession()
{
    d->teardownRequested = false;

    if (d->prepped) {
        return;
    }

    d->prepped = true;
    if (d->singleMode) {
        if (d->currentSingleRunner) {
            emit d->currentSingleRunner->prepare();
            d->singleRunnerPrepped = true;
        }
    } else {
        foreach (AbstractRunner *runner, d->runners) {
#ifdef MEASURE_PREPTIME
            QTime t;
            t.start();
#endif
            emit runner->prepare();
#ifdef MEASURE_PREPTIME
#ifndef NDEBUG
            // qDebug() << t.elapsed() << runner->name();
#endif
#endif
        }

        d->allRunnersPrepped = true;
    }
}

void RunnerManager::matchSessionComplete()
{
    if (!d->prepped) {
        return;
    }

    d->teardownRequested = true;
    d->checkTearDown();
}

void RunnerManager::launchQuery(const QString &term)
{
    launchQuery(term, QString());
}

void RunnerManager::launchQuery(const QString &untrimmedTerm, const QString &runnerName)
{
    setupMatchSession();
    QString term = untrimmedTerm.trimmed();

    setSingleModeRunnerId(runnerName);
    setSingleMode(d->currentSingleRunner);
    if (!runnerName.isEmpty() && !d->currentSingleRunner) {
        reset();
        return;
    }

    if (term.isEmpty()) {
        if (d->singleMode && d->currentSingleRunner->defaultSyntax()) {
            term = d->currentSingleRunner->defaultSyntax()->exampleQueries().first().remove(QRegExp(":q:"));
        } else {
            reset();
            return;
        }
    }

    if (d->context.query() == term) {
        // we already are searching for this!
        return;
    }

    if (!d->singleMode && d->runners.isEmpty()) {
        d->loadRunners();
    }

    reset();
//    qDebug() << "runners searching for" << term << "on" << runnerName;
    d->context.setQuery(term);

    QHash<QString, AbstractRunner*> runable;

    //if the name is not empty we will launch only the specified runner
    if (d->singleMode) {
        runable.insert(QString(), d->currentSingleRunner);
        d->context.setSingleRunnerQueryMode(true);
    } else {
        runable = d->runners;
    }

    foreach (Plasma::AbstractRunner *r, runable) {
        if (r->isMatchingSuspended()) {
            continue;
        }

        d->startJob(r);
    }

    // Start timer to unblock slow runners
    d->delayTimer.start(RunnerManagerPrivate::slowRunDelay);
}

QString RunnerManager::query() const
{
    return d->context.query();
}

void RunnerManager::reset()
{
    // If ThreadWeaver is idle, it is safe to clear previous jobs
    if (Queue::instance()->isIdle()) {
        d->oldSearchJobs.clear();
    } else {
        for (auto it = d->searchJobs.constBegin(); it != d->searchJobs.constEnd(); ++it) {
            Queue::instance()->dequeue((*it));
        }
        d->oldSearchJobs += d->searchJobs;
    }

    d->searchJobs.clear();

    if (d->deferredRun.isEnabled()) {
        //qDebug() << "job actually done, running now **************";
        QueryMatch tmpRun = d->deferredRun;
        d->deferredRun = QueryMatch(0);
        tmpRun.run(d->context);
    }

    d->context.reset();
}

} // Plasma namespace


#include "moc_runnermanager.cpp"
