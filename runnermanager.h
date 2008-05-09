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

#ifndef RUNNERMANAGER_H
#define RUNNERMANAGER_H

#include <QtCore/QList>
#include <QtCore/QObject>

#include <plasma/plasma_export.h>
#include "abstractrunner.h"

namespace Plasma
{
    class QueryMatch;
    class AbstractRunner;
    class RunnerContext; 

/**
 * @short The RunnerManager class decides what installed runners are runnable,
 *        and their ratings. It is the main proxy to the runners.
 */
class PLASMA_EXPORT RunnerManager : public QObject
{
    Q_OBJECT


    public:
        explicit RunnerManager(QObject *parent=0);
        explicit RunnerManager(KConfigGroup& config, QObject *parent=0);
        ~RunnerManager();

        /**
         * Finds and returns a loaded runner or NULL
         * @arg name the name of the runner
         * @return Pointer to the runner 
         */
        AbstractRunner* runner(const QString &name) const;

        /**
         * Retrieves the current context
         * @return pointer to the current context
         */
        RunnerContext* searchContext() const;

        /**
         * Retrieves all available matches found so far for the previously launched query 
         * @return List of matches
         */
        QList<QueryMatch> matches() const;

        /**
         * Runs a given match
         * @arg pointer to the match to be executed
         */
         void run(const QueryMatch &match);

    public Q_SLOTS:
        /**
         * Launch a query, this will create threads and return inmediately. 
         * When the information will be available can be known using the 
         * matchesChanged signal.
         *
         * @arg term the term we want to find matches for
         * @arg runner optional, if only one specific runner is to be used 
         */
        void launchQuery(const QString &term, const QString & runnerName=QString());

        /**
         * Execute a query, this method will only return when the query is executed
         * This means that the method may be dangerous as it wait a variable amount
         * of time for the runner to finish.
         * The runner parameter is mandatory, to avoid launching unwanted runners.
         * @arg term the term we want to find matches for
         * @arg runner the runner we will use, it is mandatory
         * @return 0 if nothing was launched, 1 if launched.
         */
        bool execQuery(const QString &term, const QString & runnerName);

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

        class Private;
        Private * const d;
};

}

#endif
