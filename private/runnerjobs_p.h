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

#ifndef PLASMA_RUNNERJOBS_P_H
#define PLASMA_RUNNERJOBS_P_H

#include <QHash>
#include <QMutex>
#include <QSet>

#include <Weaver/Job.h>
#include <Weaver/QueuePolicy.h>

#include "abstractrunner.h"

using ThreadWeaver::Job;

class QTimer;

namespace Plasma {
// Queue policies

// QueuePolicy that only allows a job to be executed after
// waiting in the queue for the specified timeout
class DelayedRunnerPolicy : public ThreadWeaver::QueuePolicy
{
public:
    ~DelayedRunnerPolicy();

    static DelayedRunnerPolicy &instance();

    bool canRun(Job *job);
    void free(Job *job);
    void release(Job *job);
    void destructed(Job *job);
private:
    DelayedRunnerPolicy();
    QMutex m_mutex;
};

// QueuePolicy that limits the instances of a particular runner
class DefaultRunnerPolicy : public ThreadWeaver::QueuePolicy
{
public:
    ~DefaultRunnerPolicy();

    static DefaultRunnerPolicy &instance();

    void setCap(int cap)
    {
        m_cap = cap;
    }
    int cap() const
    {
        return m_cap;
    }

    bool canRun(Job *job);
    void free(Job *job);
    void release(Job *job);
    void destructed(Job *job);
private:
    DefaultRunnerPolicy();

    int m_cap;
    QHash<QString, int> m_runCounts;
    QMutex m_mutex;
};

/* ThreadWeaver work around:
 * There is no method exposed that allows us to inform
 * ThreadWeaver that a previously unavailable job is now
 * available; thus, we use an empty job to wake up the threads
 */
class DummyJob : public ThreadWeaver::Job
{
    public:
        DummyJob(QObject *parent) : Job(parent) {}
        ~DummyJob() {}
    private:
        void run() {}
};

/*
 * FindMatchesJob class
 * Class to run queries in different threads
 */
class FindMatchesJob : public Job
{
public:
    FindMatchesJob(Plasma::AbstractRunner *runner,
                   Plasma::RunnerContext *context, QObject *parent = 0);
    ~FindMatchesJob();

    int priority() const;
    Plasma::AbstractRunner* runner() const;

    QTimer* delayTimer() const;
    void setDelayTimer(QTimer *timer);

protected:
    void run();

private:
    Plasma::RunnerContext m_context;
    Plasma::AbstractRunner *m_runner;
    QTimer *m_timer;
};

class DelayedJobCleaner : public QObject
{
public:
    DelayedJobCleaner(QSet<FindMatchesJob*> jobs, ThreadWeaver::WeaverInterface *weaver);

private Q_SLOTS:
    void jobDone(ThreadWeaver::Job*);
    void checkIfFinished();
    //connect(ThreadWeaver::instance(), SIGNAL(finished()), this, SLOT(checkIfFinished()));

private:
    ThreadWeaver::WeaverInterface *m_weaver;
    QSet<FindMatchesJob*> m_jobs;
};

}

#endif // PLASMA_RUNNERJOBS_P_H
