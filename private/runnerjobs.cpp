/*
 *   Copyright (C) 2007, 2009 Ryan P. Bitanga <ryan.bitanga@gmail.com>
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

#include "runnerjobs_p.h"

#include <QTimer>

#include <kdebug.h>

//#include <Weaver/DebuggingAids.h>
#include <Weaver/ThreadWeaver.h>

#include "runnermanager.h"
#include "plasma/querymatch.h"

using ThreadWeaver::Job;
using ThreadWeaver::Weaver;

namespace Plasma {

DelayedRunnerPolicy::DelayedRunnerPolicy()
    : QueuePolicy()
{}

DelayedRunnerPolicy::~DelayedRunnerPolicy()
{}

DelayedRunnerPolicy& DelayedRunnerPolicy::instance()
{
    static DelayedRunnerPolicy policy;
    return policy;
}

bool DelayedRunnerPolicy::canRun(Job *job)
{
    FindMatchesJob *aJob = static_cast<FindMatchesJob*>(job);
    if (QTimer *t = aJob->delayTimer()) {
        // If the timer is active, the required delay has not been reached
        //kDebug() << "delayed timer" << aJob->runner()->name() << !t->isActive();
        return !t->isActive();
    }

    return true;
}

void DelayedRunnerPolicy::free(Job *job)
{
    Q_UNUSED(job)
}

void DelayedRunnerPolicy::release(Job *job)
{
    free(job);
}

void DelayedRunnerPolicy::destructed(Job *job)
{
    Q_UNUSED(job)
}

DefaultRunnerPolicy::DefaultRunnerPolicy()
    : QueuePolicy(),
      m_cap(2)
{}

DefaultRunnerPolicy::~DefaultRunnerPolicy()
{}

DefaultRunnerPolicy& DefaultRunnerPolicy::instance()
{
    static DefaultRunnerPolicy policy;
    return policy;
}

bool DefaultRunnerPolicy::canRun(Job *job)
{
    Plasma::AbstractRunner *runner = static_cast<FindMatchesJob*>(job)->runner();
    QMutexLocker l(&m_mutex);

    if (m_runCounts[runner->name()] > m_cap) {
        return false;
    } else {
        ++m_runCounts[runner->name()];
        return true;
    }
}

void DefaultRunnerPolicy::free(Job *job)
{
    Plasma::AbstractRunner *runner = static_cast<FindMatchesJob*>(job)->runner();
    QMutexLocker l(&m_mutex);

    --m_runCounts[runner->name()];
}

void DefaultRunnerPolicy::release(Job *job)
{
    free(job);
}

void DefaultRunnerPolicy::destructed(Job *job)
{
    Q_UNUSED(job)
}

////////////////////
// Jobs
////////////////////

FindMatchesJob::FindMatchesJob(Plasma::AbstractRunner *runner,
                               Plasma::RunnerContext *context, QObject *parent)
    : ThreadWeaver::Job(parent),
      m_context(*context, 0),
      m_runner(runner),
      m_timer(0)
{
    if (runner->speed() == Plasma::AbstractRunner::SlowSpeed) {
        assignQueuePolicy(&DelayedRunnerPolicy::instance());
    } else {
        assignQueuePolicy(&DefaultRunnerPolicy::instance());
    }
}

FindMatchesJob::~FindMatchesJob()
{
}

QTimer* FindMatchesJob::delayTimer() const
{
    return m_timer;
}

void FindMatchesJob::setDelayTimer(QTimer *timer)
{
    m_timer = timer;
}

void FindMatchesJob::run()
{
//     kDebug() << "Running match for " << m_runner->objectName()
//              << " in Thread " << thread()->id() << endl;
    if (m_context.isValid()) {
        m_runner->performMatch(m_context);
    }
}

int FindMatchesJob::priority() const
{
    return m_runner->priority();
}

Plasma::AbstractRunner* FindMatchesJob::runner() const
{
    return m_runner;
}

DelayedJobCleaner::DelayedJobCleaner(QSet<FindMatchesJob*> jobs, ThreadWeaver::WeaverInterface *weaver)
    : QObject(weaver),
      m_weaver(weaver),
      m_jobs(jobs)
{
    connect(m_weaver, SIGNAL(finished()), this, SLOT(checkIfFinished()));

    foreach (FindMatchesJob *job, m_jobs) {
        connect(job, SIGNAL(done(ThreadWeaver::Job*)), this, SLOT(jobDone(ThreadWeaver::Job*)));
    }
}

void DelayedJobCleaner::jobDone(ThreadWeaver::Job *job)
{
    FindMatchesJob *runJob = dynamic_cast<FindMatchesJob *>(job);

    if (!runJob) {
        return;
    }

    m_jobs.remove(runJob);
    runJob->deleteLater();

    if (m_jobs.isEmpty()) {
        deleteLater();
    }
}

void DelayedJobCleaner::checkIfFinished()
{
    if (m_weaver->isIdle()) {
        qDeleteAll(m_jobs);
        m_jobs.clear();
        deleteLater();
    }
}


} // Plasma namespace

// #include "runnerjobs.moc"
