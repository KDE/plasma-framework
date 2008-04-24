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

#ifndef SEARCHCONTEXT_H
#define SEARCHCONTEXT_H

#include <QtCore/QList>
#include <QtCore/QObject>

#include <plasma/plasma_export.h>

class KCompletion;

namespace Plasma
{

class SearchMatch;
class AbstractRunner;

/**
 * @short The SearchContext class provides information related to a search,
 *        including the search term, metadata on the search term and collected
 *        matches.
 */
class PLASMA_EXPORT SearchContext : public QObject
{
    Q_OBJECT

    public:
        enum Type { UnknownType = 0,
                    Directory,
                    File,
                    NetworkLocation,
                    Executable,
                    ShellCommand,
                    Help
                  };

        enum DataPolicy { Shared = 0,
                          SingleConsumer
                        };

        explicit SearchContext(QObject *parent = 0, DataPolicy policy = Shared);

        /**
         * Constructs a SearchContext with a DataPolicy of SingleConsumer that
         * contains the search metadata (though none of the currently registered
         * matches) from the passed in SearchContext. Primarily useful for creating
         * a thread-local copy of a Shared SearchContext.
         */
        SearchContext(QObject *parent, const SearchContext& other);
        ~SearchContext();


        /**
         * Sets the search term for this object and attempts to determine
         * the type of the search.
         * This clears all current matches in the process.
         */
        void resetSearchTerm(const QString&);

        /**
         * Sets the search term for this object and attempts to determine
         * the type of the search.
         */
        void setSearchTerm(const QString&);

        /**
         * @return the current search term.
         */
        QString searchTerm() const;

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
         * @return a completion object that can be used with UI elements
         */
        KCompletion* completionObject() const;

        /**
         * Adds an item to the completion object.
         */
        void addStringCompletion(const QString& completion);

        /**
         * Adds multiple items to the completion object.
         */
        void addStringCompletions(const QStringList& completions);

        /**
         * Appends lists of matches to the list of matches.
         * The SearchContext takes over ownership of the matches on successful addition.
         *
         * This method is thread safe and causes the matchesChanged() signal to be emitted.
         *
         * @return true if matches were added, false if matches were e.g. outdated
         */
        bool addMatches(const QString& term, const QList<SearchMatch *> &matches);

        /**
         * Appends a match to the existing list of matches.
         * The SearchContext takes over ownership of the match on successful addition.
         *
         * If you are going to be adding multiple matches, it is better to use
         * addMatches instead.
         *
         * @arg term the search term that this match was generated for
         * @arg match the match to add
         *
         * @return true if the match was added, false otherwise.
         */
        bool addMatch(const QString &term, SearchMatch *match);

        /**
         * Takes the matches from this SearchContext and copies to them another.
         * If successful, the matches are removed from this SearchContext and
         * ownership passed to the other SearchContext
         *
         * @arg other the SearchContext to move this object's Matches to
         * @return true if matches were added, false if matches were e.g. outdated
         */
        bool moveMatchesTo(SearchContext &other);

        /**
         * Retrieves all available matches for the current search term.
         */
        QList<SearchMatch *> matches() const;

        /**
         * Determines type of query
         */
        void determineType();

        /**
         * Removes all matches from this SearchContext.
         */
        void removeAllMatches();

    Q_SIGNALS:
        void matchesChanged();

    private:
        class Private;
        Private * const d;
};

}

#endif
