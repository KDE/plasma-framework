/*
 *   Copyright 2009 Aaron Seigo <aseigo@kde.org>
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

#ifndef PLASMA_RUNNERSYNTAX_H
#define PLASMA_RUNNERSYNTAX_H

#include <QtCore/QStringList>

#include <plasma/plasma.h>

namespace Plasma
{

class RunnerSyntaxPrivate;
/**
 * @class RunnerSyntax
 * @since 4.3
 *
 * Represents a query prototype that the runner accepts. These can be
 * created and registered with AbstractRunner::addSyntax(Syntax &) to
 * allow applications to show to the user what the runner is currently
 * capable of doing
 */
class PLASMA_EXPORT RunnerSyntax
{
    public:
        /**
         * Constructs a simple syntax object
         *
         * @arg exampleQuery an example of the query, with :q: placed wherever
         *                   search term text might appear. e.g. if the runner
         *                   accepts "keyword some random text" then the value
         *                   of this parameter should be "keyword :q:"
         * @arg descrition A description of what the described syntax does from
         *                 the user's point of view.
         */
        RunnerSyntax(const QString &exampleQuery, const QString &description);

        /**
         * Copy constructor
         */
        RunnerSyntax(const RunnerSyntax &other);

        ~RunnerSyntax();

        /**
         * Assignment operator
         */
        RunnerSyntax &operator=(const RunnerSyntax &rhs);

        /**
         * Adds a synonymous example query to this Syntax. Some runners may
         * accept multiple formulations of keywords to trigger the same behaviour.
         * This allows the runner to show these relationships by grouping the
         * example queries into one Syntax object
         *
         * @arg exampleQuery an example of the query, with :q: placed wherever
         *                   search term text might appear. e.g. if the runner
         *                   accepts "keyword some random text" then the value
         *                   of this parameter should be "keyword :q:"
         */
        void addExampleQuery(const QString &exampleQuery);

        /**
         * @return the example queries associated with this Syntax object
         */
        QStringList exampleQueries() const;

        /**
         * @return the example queries associated with this Syntax object, with
         * the searchTermDescription replacing instances of :q:. Used for showing
         * the queries in the user interface.
         */
        QStringList exampleQueriesWithTermDescription() const;

        /**
         * Sets the description for the syntax, describing what it does from
         * the user's point of view.
         */
        void setDescription(const QString &description);

        /**
         * @return the description of what the syntax does from the user's
         *         point of view
         */
        QString description() const;

        /**
         * Sets the text that should be used to replace instances of :q:
         * in the text. By default this is the generic phrase "search term".
         * If the syntax expects a specific kind of input, it may be defined
         * here. A syntax used by a runner that changes the brightness of the display 
         * may set this to "brightness" for instance.
         */
        void setSearchTermDescription(const QString &description);

        /**
         * @return a description of the search term for this syntax
         */
        QString searchTermDescription() const;

    private:
        RunnerSyntaxPrivate *const d;
};

} // namespace Plasma

#endif // multiple inclusion guard

