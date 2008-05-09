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

#ifndef RUNNERCONTEXT_H
#define RUNNERCONTEXT_H

#include <QtCore/QList>
#include <QtCore/QObject>
#include <QtCore/QSharedDataPointer>

#include <plasma/plasma_export.h>

class KCompletion;

namespace Plasma
{

class QueryMatch;
class AbstractRunner;

/**
 * @short The RunnerContext class provides information related to a search,
 *        including the search term, metadata on the search term and collected
 *        matches.
 */
class PLASMA_EXPORT RunnerContext : public QObject
{
    Q_OBJECT

    public:
        enum Type { None = 0,
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
        explicit RunnerContext(RunnerContext& other, QObject *parent = 0);

        ~RunnerContext();


        /**
         * Resets the search term for this object.
         * This removes all current matches in the process.
         */
        void reset();

        /**
         * Sets the query term for this object and attempts to determine
         * the type of the search.
         */
        void setQuery(const QString& term);

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
         * Appends lists of matches to the list of matches.
         * The RunnerContext takes over ownership of the matches on successful addition.
         *
         * This method is thread safe and causes the matchesChanged() signal to be emitted.
         *
         * @return true if matches were added, false if matches were e.g. outdated
         */
        bool addMatches(const QString& term, const QList<QueryMatch> &matches);

        /**
         * Appends a match to the existing list of matches.
         * The RunnerContext takes over ownership of the match on successful addition.
         *
         * If you are going to be adding multiple matches, use addMatches instead.
         *
         * @arg term the search term that this match was generated for
         * @arg match the match to add
         *
         * @return true if the match was added, false otherwise.
         */
        bool addMatch(const QString &term, const QueryMatch &match);

        /**
         * Takes the matches from this RunnerContext and copies to them another.
         * If successful, the matches are removed from this RunnerContext and
         * ownership passed to the other RunnerContext
         *
         * @arg other the RunnerContext to move this object's Matches to
         * @return true if matches were added, false if matches were e.g. outdated
         */
        bool moveMatchesTo(RunnerContext &other);

        /**
         * Retrieves all available matches for the current search term.
         */
        QList<QueryMatch> matches() const;

    Q_SIGNALS:
        void matchesChanged();

    private:
        class Private;
        QExplicitlySharedDataPointer<Private> d;
};

}

Q_DECLARE_OPERATORS_FOR_FLAGS(Plasma::RunnerContext::Types)

#endif
