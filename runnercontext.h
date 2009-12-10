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

#ifndef PLASMA_RUNNERCONTEXT_H
#define PLASMA_RUNNERCONTEXT_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>

#include <plasma/plasma_export.h>

class KCompletion;
class KConfigGroup;

namespace Plasma
{

class QueryMatch;
class AbstractRunner;
class RunnerContextPrivate;

/**
 * @class RunnerContext plasma/runnercontext.h <Plasma/RunnerContext>
 *
 * @short The RunnerContext class provides information related to a search,
 *        including the search term, metadata on the search term and collected
 *        matches.
 */
class PLASMA_EXPORT RunnerContext : public QObject
{
    Q_OBJECT

    public:
        enum Type {
            None = 0,
            UnknownType = 1,
            Directory = 2,
            File = 4,
            NetworkLocation = 8,
            Executable = 16,
            ShellCommand = 32,
            Help = 64,
            FileSystem = Directory | File | Executable | ShellCommand
        };

        Q_DECLARE_FLAGS(Types, Type)

        explicit RunnerContext(QObject *parent = 0);

        /**
         * Copy constructor
         */
        RunnerContext(RunnerContext &other, QObject *parent = 0);

        /**
         * Assignment operator
         * @since 4.4
         */
        RunnerContext &operator=(const RunnerContext &other);

        ~RunnerContext();

        /**
         * Resets the search term for this object.
         * This removes all current matches in the process and
         * turns off single runner query mode.
         */
        void reset();

        /**
         * Sets the query term for this object and attempts to determine
         * the type of the search.
         */
        void setQuery(const QString &term);

        /**
         * @return the current search query term.
         */
        QString query() const;

        /**
         * The type of item the search term might refer to.
         * @see Type
         */
        Type type() const;

        /**
         * The mimetype that the search term refers to, if discoverable.
         *
         * @return QString() if the mimetype can not be determined, otherwise
         *         the mimetype of the object being referred to by the search
         *         string.
         */
        QString mimeType() const;

        /**
         * @returns true if this context is no longer valid and therefore
         * matching using it should abort. Most useful as an optimization technique
         * inside of AbstractRunner subclasses in the match method, e.g.:
         *
         * while (.. a possibly large iteration) {
         *     if (!context.isValid()) {
         *         return;
         *     }
         *
         *     ... some processing ...
         * }
         *
         * While not required to be used within runners, it provies a nice way
         * to avoid unnecessary processing in runners that may run for an extended
         * period (as measured in 10s of ms) and therefore improve the user experience. 
         * @since 4.2.3
         */
        bool isValid() const;

         /**
         * Appends lists of matches to the list of matches.
         *
         * This method is thread safe and causes the matchesChanged() signal to be emitted.
         *
         * @return true if matches were added, false if matches were e.g. outdated
         */
        // trueg: what do we need the term for? It is stored in the context anyway! Plus: matches() does not have a term parameter!
        //        plus: it is Q_UNUSED
        bool addMatches(const QString &term, const QList<QueryMatch> &matches);

        /**
         * Appends a match to the existing list of matches.
         *
         * If you are going to be adding multiple matches, use addMatches instead.
         *
         * @arg term the search term that this match was generated for.
         * @arg match the match to add
         *
         * @return true if the match was added, false otherwise.
         */
        // trueg: what do we need the term for? It is stored in the context anyway! Plus: matches() does not have a term parameter!
        //        plus: it is Q_UNUSED
        bool addMatch(const QString &term, const QueryMatch &match);

        /**
         * Removes a match from the existing list of matches.
         *
         * If you are going to be removing multiple matches, use removeMatches instead.
         *
         * @arg matchId the id of match to remove
         *
         * @return true if the match was removed, false otherwise.
	 * @since 4.4
         */
	bool removeMatch(const QString matchId);

        /**
         * Removes lists of matches from the existing list of matches.
         *
         * This method is thread safe and causes the matchesChanged() signal to be emitted.
         *
         * @arg matchIdList the list of matches id to remove
         *
         * @return true if at least one match was removed, false otherwise.
         * @since 4.4
         */
        bool removeMatches(const QStringList matchIdList);

        /**
         * Retrieves all available matches for the current search term.
         *
         * @return a list of matches
         */
        QList<QueryMatch> matches() const;

        /**
         * Retrieves a match by id.
         *
         * @param id the id of the match to return
         * @return the match associated with this id, or an invalid QueryMatch object
         *         if the id does not eixst
         */
        QueryMatch match(const QString &id) const;

        /**
         * Sets single runner query mode. Note that a call to reset() will
         * turn off single runner query mode.
         *
         * @see reset()
         * @since 4.4
         */
         void setSingleRunnerQueryMode(bool enabled);

        /**
         * @return true if the current query is a single runner query
         * @since 4.4
         */
        bool singleRunnerQueryMode() const;

        /**
         * Sets the launch counts for the associated match ids
         *
         * If a runner adds a match to this context, the context will check if the
         * match id has been launched before and increase the matches relevance
         * correspondingly. In this manner, any front end can implement adaptive search
         * by sorting items according to relevance.
         *
         * @param config the config group where launch data was stored
         */
        void restore(const KConfigGroup &config);

        /**
         * @param config the config group where launch data should be stored
         */
        void save(KConfigGroup &config);

        /**
         * Run a match using the information from this context
         *
         * The context will also keep track of the number of times the match was
         * launched to sort future matches according to user habits
         *
         * @param match the match to run
         */
        void run(const QueryMatch &match);

    Q_SIGNALS:
        void matchesChanged();

    private:
        QExplicitlySharedDataPointer<RunnerContextPrivate> d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::RunnerContext::Types)

#endif
