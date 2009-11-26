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
#include <Weaver/State.h>
#include <Weaver/Thread.h>
#include <Weaver/ThreadWeaver.h>

#include "private/runnerjobs_p.h"
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
        matchChangeTimer.start(50);
    }

    void matchesChanged()
    {
        emit q->matchesChanged(context.matches());
    }

    void loadConfiguration()
    {
        KConfigGroup config = configGroup();

        //The number of threads used scales with the number of processors.
        const int numProcs =
            qMax(Solid::Device::listFromType(Solid::DeviceInterface::Processor).count(), 1);
        //This entry allows to define a hard upper limit independent of the number of processors.
        const int maxThreads = config.readEntry("maxThreads", 16);
        const int numThreads = qMin(maxThreads, 2 + ((numProcs - 1) * 2));
        //kDebug() << "setting up" << numThreads << "threads for" << numProcs << "processors";
        if (numThreads > Weaver::instance()->maximumNumberOfThreads()) {
            Weaver::instance()->setMaximumNumberOfThreads(numThreads);
        }
        // Limit the number of instances of a single normal speed runner and all of the slow runners
        // to half the number of threads
        const int cap = qMax(2, numThreads/2);
        DefaultRunnerPolicy::instance().setCap(cap);

        context.restore(config);
    }

    KConfigGroup configGroup()
    {
        return conf.isValid() ? conf : KConfigGroup(KGlobal::config(), "PlasmaRunnerManager");
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
        if (!singleMode || singleModeRunnerId.isEmpty() ||
            !enabledSingleRunnerIds.contains(singleModeRunnerId)) {
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
            QString api = service->property("X-Plasma-API").toString();
            QString error;

            if (api.isEmpty()) {
                QVariantList args;
                args << service->storageId();
                if (Plasma::isPluginVersionCompatible(KPluginLoader(*service).pluginVersion())) {
                    currentSingleRunner = service->createInstance<AbstractRunner>(q, args, &error);
                }
            } else {
                currentSingleRunner = new AbstractRunner(q, service->storageId());
            }

            if (currentSingleRunner) {
                QMetaObject::invokeMethod(currentSingleRunner, "init");
                emit currentSingleRunner->prepare();
                singleRunnerWasLoaded = true;
            }
        }
    }

    void loadRunners()
    {
        KConfigGroup config = configGroup();
        KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");

        const bool loadAll = config.readEntry("loadAll", false);
        QStringList whiteList = config.readEntry("pluginWhiteList", QStringList());
        const bool noWhiteList = whiteList.isEmpty();
        KConfigGroup pluginConf;
        if (conf.isValid()) {
            pluginConf = KConfigGroup(&conf, "Plugins");
        } else {
            pluginConf = KConfigGroup(KGlobal::config(), "Plugins");
        }

        enabledSingleRunnerIds.clear();

        foreach (const KService::Ptr &service, offers) {
            //kDebug() << "Loading runner: " << service->name() << service->storageId();
            QString tryExec = service->property("TryExec", QVariant::String).toString();
            //kDebug() << "TryExec is" << tryExec;
            if (!tryExec.isEmpty() && KStandardDirs::findExe(tryExec).isEmpty()) {
                // we don't actually have this application!
                continue;
            }

            KPluginInfo description(service);
            QString runnerName = description.pluginName();
            description.load(pluginConf);

            const bool loaded = runners.contains(runnerName);
            const bool selected = loadAll ||
                            (description.isPluginEnabled() && (noWhiteList || whiteList.contains(runnerName)));

            const bool singleQueryModeEnabled = service->property("SingleRunnerQueryMode", QVariant::Bool).toBool();

            if (singleQueryModeEnabled) {
                enabledSingleRunnerIds.insert(runnerName, description.name());
            }

            //kDebug() << loadAll << description.isPluginEnabled() << noWhiteList << whiteList.contains(runnerName);
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
                        kDebug() << "================= loading runner:" << service->name() << "=================";

                        /*
                        foreach (const RunnerSyntax &syntax, runner->syntaxes()) {
                            kDebug() << syntax.exampleQueriesWithTermDescription().join(", ") << " ==> " << syntax.description();
                        }
                        */

                        QMetaObject::invokeMethod(runner, "init");
                        runners.insert(runnerName, runner);
                    } else {
                        kDebug() << "failed to load runner:" << service->name()
                                 << ". error reported:" << error;
                        continue;
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
        FindMatchesJob *runJob = dynamic_cast<FindMatchesJob *>(job);

        if (!runJob) {
            return;
        }

        if (deferredRun.isEnabled() && runJob->runner() == deferredRun.runner()) {
            //kDebug() << "job actually done, running now **************";
            QueryMatch tmpRun = deferredRun;
            deferredRun = QueryMatch(0);
            tmpRun.run(context);
        }

        searchJobs.remove(runJob);
        oldSearchJobs.remove(runJob);
        runJob->deleteLater();

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
        //kDebug() << prepped << teardownRequested << searchJobs.count() << oldSearchJobs.count();

        if (!prepped || !teardownRequested) {
            return;
        }

        if (Weaver::instance()->isIdle()) {
            qDeleteAll(searchJobs);
            searchJobs.clear();
            qDeleteAll(oldSearchJobs);
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

            prepped = false;
            teardownRequested = false;
        }
    }

    void unblockJobs()
    {
        // WORKAROUND: Queue an empty job to force ThreadWeaver to awaken threads
        if (searchJobs.isEmpty() && Weaver::instance()->isIdle()) {
            qDeleteAll(oldSearchJobs);
            oldSearchJobs.clear();
            checkTearDown();
            return;
        }

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
    QHash<QString, QString> enabledSingleRunnerIds;
    AbstractRunner* currentSingleRunner;
    QSet<FindMatchesJob*> searchJobs;
    QSet<FindMatchesJob*> oldSearchJobs;
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
        new DelayedJobCleaner(d->searchJobs + d->oldSearchJobs, Weaver::instance());
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
    qDeleteAll(d->runners);
    d->runners.clear();

    KConfigGroup config = d->configGroup();
    config.writeEntry("pluginWhiteList", runners);
}

QStringList RunnerManager::allowedRunners() const
{
    KConfigGroup config = d->configGroup();
    return config.readEntry("pluginWhiteList", QStringList());
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

QStringList RunnerManager::enabledSingleModeRunnerIds() const
{
    return d->enabledSingleRunnerIds.keys();
}

QString RunnerManager::runnerName(const QString &id) const
{
    if (runner(id)) {
        return runner(id)->name();
    } else {
        return d->enabledSingleRunnerIds.value(id, QString());
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
            kDebug() << "deferred run";
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
            emit runner->prepare();
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

    if (!runnerName.isEmpty()) {
        setSingleModeRunnerId(runnerName);
        setSingleMode(true);
    }

    if (term.isEmpty()) {
        if (d->singleMode && d->currentSingleRunner && d->currentSingleRunner->defaultSyntax()) {
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

    if (d->singleMode && !d->currentSingleRunner) {
        reset();
        return;
    }

    if (d->runners.isEmpty()) {
        d->loadRunners();
    }

    reset();
//    kDebug() << "runners searching for" << term << "on" << runnerName;
    d->context.setQuery(term);

    AbstractRunner::List runable;

    //if the name is not empty we will launch only the specified runner
    if (d->singleMode && d->currentSingleRunner) {
        runable.append(d->currentSingleRunner);
        d->context.setSingleRunnerQueryMode(true);
    } else {
        runable = d->runners.values();
    }

    foreach (Plasma::AbstractRunner *r, runable) {
        if ((r->ignoredTypes() & d->context.type()) == 0) {
//            kDebug() << "launching" << r->name();
            FindMatchesJob *job = new FindMatchesJob(r, &d->context, Weaver::instance());
            connect(job, SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(jobDone(ThreadWeaver::Job*)));
            if (r->speed() == AbstractRunner::SlowSpeed) {
                job->setDelayTimer(&d->delayTimer);
            }
            Weaver::instance()->enqueue(job);
            d->searchJobs.insert(job);
        }
    }

    // Start timer to unblock slow runners
    d->delayTimer.start(RunnerManagerPrivate::slowRunDelay);
}

bool RunnerManager::execQuery(const QString &term)
{
    return execQuery(term, QString());
}

bool RunnerManager::execQuery(const QString &untrimmedTerm, const QString &runnerName)
{
    QString term = untrimmedTerm.trimmed();

    if (term.isEmpty()) {
        reset();
        return false;
    }

    if (d->runners.isEmpty()) {
        d->loadRunners();
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
        qDeleteAll(d->oldSearchJobs);
        d->oldSearchJobs.clear();
    } else {
        Weaver::instance()->dequeue();
        d->oldSearchJobs += d->searchJobs;
    }

    d->searchJobs.clear();

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
