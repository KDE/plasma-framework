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
            : /*search(s),*/
              runner(r),
              type(SearchAction::ExactMatch),
              enabled(true),
              relevance(1)
        {
            searchTerm = s->searchTerm();
            mimetype = s->mimetype();
        }
        QString searchTerm;
        SearchContext *search;
        AbstractRunner *runner;
        SearchAction::Type type;
        QString mimetype;
        QString text;
        QIcon icon;
        QVariant data;
        bool enabled;
        qreal relevance;
};


SearchAction::SearchAction(SearchContext *search, AbstractRunner *runner)
    : d(new Private(search, runner))
{
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
    return d->mimetype;//.isEmpty() ? d->search->mimetype() : d->mimetype;
}

QString SearchAction::searchTerm() const
{
    return d->searchTerm;//->searchTerm();
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

void SearchAction::setText(const QString& text)
{
    d->text = text;
}

void SearchAction::setData(const QVariant& data)
{
    d->data = data;
}

void SearchAction::setIcon(const QIcon& icon)
{
    d->icon = icon;
}

QVariant SearchAction::data() const
{
    return d->data;
}

QString SearchAction::text() const
{
    return d->text;
}

QIcon SearchAction::icon() const
{
    return d->icon;
}

void SearchAction::setEnabled( bool enabled )
{
    d->enabled = enabled;
}

bool SearchAction::isEnabled() const
{
  return d->enabled;
}

bool SearchAction::operator<(const SearchAction& other) const
{
    return d->relevance < other.d->relevance;
}

void SearchAction::exec()
{
    if(d->runner) {
    //TODO: this could be dangerous if the runner is deleted behind our backs.
        d->runner->exec(this);
    }
}

}

#include "searchaction.moc"
