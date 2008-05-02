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

#ifndef QUERYMATCH_H
#define QUERYMATCH_H

#include <plasma/plasma_export.h>

class QIcon;
class QVariant;
class QString;

namespace Plasma
{

class RunnerContext;
class AbstractRunner;

class PLASMA_EXPORT QueryMatch
{
    public:
        /**
         * The type of match. Value is important here as it is used for sorting
         */
        enum Type { CompletionMatch = 10 /**< Possible completion for the data of the query */,
                    PossibleMatch = 30 /**< Something that may match the query */,
                    InformationalMatch = 50 /**< A purely informational, non-actionable match,
                                                 such as the answer to a question or calculation*/,
                    HelperMatch = 70 /**< A match that represents an action not directly related
                                          to activating the given search term, such as a search
                                          in an external tool or a command learning trigger. Helper
                                          matches tend to be generic to the query and should not
                                          be autoactivated just because the user hits "Enter"
                                          while typing. They must be explicitly selected to 
                                          be activated, but unlike InformationalMatch cause
                                          an action to be triggered. */,
                    ExactMatch = 100 /**< An exact match to the query */};


        /**
         * Constructs a PossibleMatch associated with a given RunnerContext
         * and runner.
         *
         * @arg search the RunnerContext this match belongs to
         * @arg runner the runner this match belongs to
         */
        explicit QueryMatch(AbstractRunner *runner);
        ~QueryMatch();

        /**
         * Sets the type of match this action represents.
         */
        void setType(Type type);

        /**
         * The type of action this is. Defaults to PossibleMatch.
         */
        Type type() const;

        /**
         * Sets the relevance of this action for the search
         * it was created for.
         *
         * @param relevance a number between 0 and 1.
         */
        void setRelevance(qreal relevance);

        /**
         * The relevance of this action to the search. By default,
         * the relevance is 1.
         *
         * @return a number between 0 and 1
         */
        qreal relevance() const;

        /**
         * The runner associated with this action
         */
        AbstractRunner* runner() const;

        void setText(const QString& text);
        void setSubtext(const QString& text);
        void setData(const QVariant& data);
        void setIcon(const QIcon& icon);
        void setEnabled(bool enable);

        QString text() const;
        QString subtext() const;
        QVariant data() const;
        QIcon icon() const;
        bool isEnabled() const;

        bool operator<(const QueryMatch& other) const;

        void run(const RunnerContext *context) const;

    private:
        class Private;
        Private * const d;
};

}

#endif
