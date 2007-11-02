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

#include "searchaction.h"

#include "abstractrunner.h"

namespace Plasma
{

class SearchAction::Private
{
    public:
        Private(SearchContext* s, AbstractRunner *r)
            : search(s),
              runner(r),
              type(SearchAction::ExactMatch),
              relevance(1)
        {
        }

        SearchContext *search;
        AbstractRunner *runner;
        SearchAction::Type type;
        QString mimetype;
        qreal relevance;
};


SearchAction::SearchAction(SearchContext *search, AbstractRunner *runner)
    : QAction(search),
      d(new Private(search, runner))
{
    connect(this, SIGNAL(triggered(bool)), this, SLOT(exec()));
}

SearchAction::~SearchAction()
{
    delete d;
}

void SearchAction::setType(Type type)
{
    d->type = type;
}

SearchAction::Type SearchAction::type() const
{
    return d->type;
}

void SearchAction::setMimetype(const QString &mimetype)
{
    d->mimetype = mimetype;
}

QString SearchAction::mimetype() const
{
    return d->mimetype.isEmpty() ? d->search->mimetype() : d->mimetype;
}

QString SearchAction::searchTerm() const
{
    return d->search->searchTerm();
}

void SearchAction::setRelevance(qreal relevance)
{
    d->relevance = qMax(0.0, qMin(1.0, relevance));
}

qreal SearchAction::relevance() const
{
    return d->relevance;
}

AbstractRunner* SearchAction::runner() const
{
    return d->runner;
}

bool SearchAction::operator<(const SearchAction& other) const
{
    return d->relevance < other.d->relevance;
}

void SearchAction::exec()
{
    //TODO: this could be dangerous if the runner is deleted behind our backs.
    d->runner->exec(this);
}

}

#include "searchaction.moc"
