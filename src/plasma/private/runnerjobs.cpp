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

#include <QDebug>

#include "runnermanager.h"
#include "plasma/querymatch.h"

using ThreadWeaver::Job;
using ThreadWeaver::Queue;

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

bool DelayedRunnerPolicy::canRun(ThreadWeaver::JobPointer job)
{
    QSharedPointer<FindMatchesJob> aJob(job.dynamicCast<FindMatchesJob>());
    if (QTimer *t = aJob->delayTimer()) {
        // If the timer is active, the required delay has not been reached
        //qDebug() << "delayed timer" << aJob->runner()->name() << !t->isActive();
        return !t->isActive();
    }

    return true;
}

void DelayedRunnerPolicy::free(ThreadWeaver::JobPointer job)
{
    Q_UNUSED(job)
}

void DelayedRunnerPolicy::release(ThreadWeaver::JobPointer job)
{
    free(job);
}

void DelayedRunnerPolicy::destructed(ThreadWeaver::JobInterface* job)
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

bool DefaultRunnerPolicy::canRun(ThreadWeaver::JobPointer job)
{
    Plasma::AbstractRunner *runner = job.dynamicCast<FindMatchesJob>()->runner();
    QMutexLocker l(&m_mutex);

    if (m_runCounts[runner->name()] > m_cap) {
        return false;
    } else {
        ++m_runCounts[runner->name()];
        return true;
    }
}

void DefaultRunnerPolicy::free(ThreadWeaver::JobPointer job)
{
    Plasma::AbstractRunner *runner = job.dynamicCast<FindMatchesJob>()->runner();
    QMutexLocker l(&m_mutex);

    --m_runCounts[runner->name()];
}

void DefaultRunnerPolicy::release(ThreadWeaver::JobPointer job)
{
    free(job);
}

void DefaultRunnerPolicy::destructed(ThreadWeaver::JobInterface* job)
{
    Q_UNUSED(job)
}

////////////////////
// Jobs
////////////////////

FindMatchesJob::FindMatchesJob(Plasma::AbstractRunner *runner,
                               Plasma::RunnerContext *context, QObject *parent)
    : ThreadWeaver::Job(),
      m_context(*context, 0),
      m_runner(runner),
      m_timer(0),
      m_decorator(new ThreadWeaver::QObjectDecorator(this, true))
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

void FindMatchesJob::run(ThreadWeaver::JobPointer, ThreadWeaver::Thread*)
{
//     qDebug() << "Running match for " << m_runner->objectName()
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

DelayedJobCleaner::DelayedJobCleaner(const QSet<QSharedPointer<FindMatchesJob> > &jobs, const QSet<AbstractRunner *> &runners)
    : QObject(Queue::instance()),
      m_weaver(Queue::instance()),
      m_jobs(jobs),
      m_runners(runners)
{
    connect(m_weaver, SIGNAL(finished()), this, SLOT(checkIfFinished()));

    for (auto it = m_jobs.constBegin(); it != m_jobs.constEnd(); ++it) {
        connect((*it)->decorator(), &ThreadWeaver::QObjectDecorator::done, this, &DelayedJobCleaner::jobDone);
    }
}

DelayedJobCleaner::~DelayedJobCleaner()
{
    qDeleteAll(m_runners);
}

void DelayedJobCleaner::jobDone(ThreadWeaver::JobPointer job)
{
    auto runJob = job.dynamicCast<FindMatchesJob>();

    if (!runJob) {
        return;
    }

    m_jobs.remove(runJob);

    if (m_jobs.isEmpty()) {
        deleteLater();
    }
}

void DelayedJobCleaner::checkIfFinished()
{
    if (m_weaver->isIdle()) {
        m_jobs.clear();
        deleteLater();
    }
}


} // Plasma namespace

