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

#include "searchmatch.h"

#include <QVariant>
#include <QStringList>
#include <QIcon>

#include <KDebug>

#include "abstractrunner.h"

namespace Plasma
{

class SearchMatch::Private
{
    public:
        Private(AbstractRunner *r)
            : runner(r),
              type(SearchMatch::ExactMatch),
              enabled(true),
              relevance(.7)
        {
        }

        QString searchTerm;
        AbstractRunner *runner;
        SearchMatch::Type type;
        QString mimetype;
        QString text;
        QIcon icon;
        QVariant data;
        bool enabled;
        qreal relevance;
};


SearchMatch::SearchMatch(AbstractRunner *runner)
    : d(new Private(runner))
{
}

SearchMatch::~SearchMatch()
{
    delete d;
}

void SearchMatch::setType(Type type)
{
    d->type = type;
}

SearchMatch::Type SearchMatch::type() const
{
    return d->type;
}

void SearchMatch::setMimetype(const QString &mimetype)
{
    d->mimetype = mimetype;
}

QString SearchMatch::mimetype() const
{
    return d->mimetype;
}
/*
QString SearchMatch::searchTerm() const
{
    return d->searchTerm;
}
*/
void SearchMatch::setRelevance(qreal relevance)
{
    d->relevance = qMax(qreal(0.0), qMin(qreal(1.0), relevance));
}

qreal SearchMatch::relevance() const
{
    return d->relevance;
}

AbstractRunner* SearchMatch::runner() const
{
    return d->runner;
}

void SearchMatch::setText(const QString& text)
{
    d->text = text;
}

void SearchMatch::setData(const QVariant& data)
{
    d->data = data;
}

void SearchMatch::setIcon(const QIcon& icon)
{
    d->icon = icon;
}

QVariant SearchMatch::data() const
{
    return d->data;
}

QString SearchMatch::text() const
{
    return d->text;
}

QIcon SearchMatch::icon() const
{
    return d->icon;
}

void SearchMatch::setEnabled( bool enabled )
{
    d->enabled = enabled;
}

bool SearchMatch::isEnabled() const
{
  return d->enabled;
}

bool SearchMatch::operator<(const SearchMatch& other) const
{
    return d->relevance < other.d->relevance;
}

void SearchMatch::exec(const SearchContext *context) const
{
    Q_ASSERT(context);

    //kDebug() << "we have" << d->searchTerm << d->mimetype;
    if (d->runner) {
        //TODO: this could be dangerous if the runner is deleted behind our backs.
        d->runner->exec(context, this);
    }
}

} // Plasma namespace

