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

#include "runnersyntax.h"

#include <klocalizedstring.h>

namespace Plasma
{

class RunnerSyntaxPrivate
{
public:
    RunnerSyntaxPrivate(const QString &s, const QString &d)
        : description(d)
    {
        exampleQueries.append(s);
    }

    QStringList exampleQueries;
    QString description;
    QString termDescription;
};

RunnerSyntax::RunnerSyntax(const QString &exampleQuery, const QString &description)
    : d(new RunnerSyntaxPrivate(exampleQuery, description))
{
}

RunnerSyntax::RunnerSyntax(const RunnerSyntax &other)
    : d(new RunnerSyntaxPrivate(*other.d))
{
}

RunnerSyntax::~RunnerSyntax()
{
    delete d;
}

RunnerSyntax &RunnerSyntax::operator=(const RunnerSyntax &rhs)
{
    *d = *rhs.d;
    return *this;
}

void RunnerSyntax::addExampleQuery(const QString &exampleQuery)
{
    d->exampleQueries.append(exampleQuery);
}

QStringList RunnerSyntax::exampleQueries() const
{
    return d->exampleQueries;
}

QStringList RunnerSyntax::exampleQueriesWithTermDescription() const
{
    QStringList queries;
    const QString termDesc('<' + searchTermDescription() + '>');
    foreach (QString query, d->exampleQueries) {
        queries << query.replace(":q:", termDesc);
    }

    return queries;
}

void RunnerSyntax::setDescription(const QString &description)
{
    d->description = description;
}

QString RunnerSyntax::description() const
{
    QString description = d->description;
    description.replace(":q:", '<' + searchTermDescription() + '>');
    return description;
}

void RunnerSyntax::setSearchTermDescription(const QString &description)
{
    d->termDescription = description;
}

QString RunnerSyntax::searchTermDescription() const
{
    if (d->termDescription.isEmpty()) {
        return i18n("search term");
    }

    return d->termDescription;
}

} // Plasma namespace

