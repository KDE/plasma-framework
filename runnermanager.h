/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2007 Ryan P. Bitanga <ryan.bitanga@gmail.com>
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

#ifndef PLASMA_RUNNERMANAGER_H
#define PLASMA_RUNNERMANAGER_H

#include <QtCore/QList>
#include <QtCore/QObject>

#include <plasma/plasma_export.h>
#include "abstractrunner.h"

class QAction;
class KConfigGroup;

namespace Plasma
{
    class QueryMatch;
    class AbstractRunner;
    class RunnerContext;
    class RunnerManagerPrivate;

/**
 * @class RunnerManager plasma/runnermanager.h <Plasma/RunnerManager>
 *
 * @short The RunnerManager class decides what installed runners are runnable,
 *        and their ratings. It is the main proxy to the runners.
 */
class PLASMA_EXPORT RunnerManager : public QObject
{
    Q_OBJECT

    public:
        explicit RunnerManager(QObject *parent=0);
        explicit RunnerManager(KConfigGroup &config, QObject *parent=0);
        ~RunnerManager();

        /**
         * Finds and returns a loaded runner or NULL
         * @arg name the name of the runner
         * @return Pointer to the runner
         */
        AbstractRunner *runner(const QString &name) const;

        /**
         * @return the currently active "single mode" runner, or null if none
         * @since 4.4
         */
        AbstractRunner *singleModeRunner() const;

        /**
         * Puts the manager into "single runner" mode using the given
         * runner; if the runner does not exist or can not be loaded then
         * the single runner mode will not be started and singleModeRunner()
         * will return NULL
         * @arg id the id of the runner to use
         * @since 4.4
         */
        void setSingleModeRunnerId(const QString &id);

        /**
         * @return the id of the runner to use in single mode
         * @since 4.4
         */
        QString singleModeRunnerId() const;

        /**
         * @return true if the manager is set to run in single runner mode
         * @since 4.4
         */
        bool singleMode() const;

        /**
         * Sets whether or not the manager is in single mode.
         *
         * @arg singleMode true if the manager should be in single mode, false otherwise
         * @since 4.4
         */
        void setSingleMode(bool singleMode);

        /**
         * Returns the translated name of a runner
         * @arg id the id of the runner
         *
         * @since 4.4
         */
        QString runnerName(const QString &id) const;

        /**
         * @return the list of all currently loaded runners
         */
        QList<AbstractRunner *> runners() const;

        /**
         * @return the names of all runners that advertise single query mode
         * @since 4.4
         */
        QStringList singleModeAdvertisedRunnerIds() const;

        /**
         * Retrieves the current context
         * @return pointer to the current context
         */
        RunnerContext *searchContext() const;

        /**
         * Retrieves all available matches found so far for the previously launched query
         * @return List of matches
         */
        QList<QueryMatch> matches() const;

        /**
         * Runs a given match
         * @arg match the match to be executed
         */
        void run(const QueryMatch &match);

        /**
         * Runs a given match
         * @arg id the id of the match to run
         */
        void run(const QString &id);

        /**
         * Retrieves the list of actions, if any, for a match
         */
        QList<QAction*> actionsForMatch(const QueryMatch &match);

        /**
         * @return the current query term
         */
        QString query() const;

        /**
         * Causes a reload of the current configuration
         */
        void reloadConfiguration();

        /**
         * Sets a whitelist for the plugins that can be loaded
         *
         * @arg plugins the plugin names of allowed runners
         * @since 4.4
         */
        void setAllowedRunners(const QStringList &runners);

        /**
         * @return the list of allowed plugins
         * @since 4.4
         */
        QStringList allowedRunners() const;

    public Q_SLOTS:
        /**
         * Call this method when the runners should be prepared for a query session.
         * Call matchSessionComplete when the query session is finished for the time
         * being.
         * @since 4.4
         * @see matchSessionComplete
         */
        void setupMatchSession();

        /**
         * Call this method when the query session is finished for the time
         * being.
         * @since 4.4
         * @see prepareForMatchSession
         */
        void matchSessionComplete();

        /**
         * Launch a query, this will create threads and return inmediately.
         * When the information will be available can be known using the
         * matchesChanged signal.
         *
         * @arg term the term we want to find matches for
         * @arg runnerId optional, if only one specific runner is to be used;
         *               providing an id will put the manager into single runner mode
         */
        void launchQuery(const QString &term, const QString &runnerId);

        /**
         * Convenience version of above
         */
        void launchQuery(const QString &term);

        /**
         * Execute a query, this method will only return when the query is executed
         * This means that the method may be dangerous as it wait a variable amount
         * of time for the runner to finish.
         * The runner parameter is mandatory, to avoid launching unwanted runners.
         * @arg term the term we want to find matches for
         * @arg runner the runner we will use, it is mandatory
         * @return 0 if nothing was launched, 1 if launched.
         */
        bool execQuery(const QString &term, const QString &runnerName);

        /**
         * Convenience version of above
         */
        bool execQuery(const QString &term);

        /**
         * Reset the current data and stops the query
         */
        void reset();

    Q_SIGNALS:
        /**
         * Emitted each time a new match is added to the list
         */
        void matchesChanged(const QList<Plasma::QueryMatch> &matches);

    private:
        Q_PRIVATE_SLOT(d, void scheduleMatchesChanged())
        Q_PRIVATE_SLOT(d, void matchesChanged())
        Q_PRIVATE_SLOT(d, void jobDone(ThreadWeaver::Job*))
        Q_PRIVATE_SLOT(d, void unblockJobs())

        RunnerManagerPrivate * const d;

        friend class RunnerManagerPrivate;
};

}

#endif
