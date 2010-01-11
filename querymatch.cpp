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

#include "querymatch.h"

#include <QAction>
#include <QIcon>
#include <QSharedData>
#include <QStringList>
#include <QVariant>
#include <QWeakPointer>

#include <kdebug.h>

#include "abstractrunner.h"

namespace Plasma
{

class QueryMatchPrivate : public QSharedData
{
    public:
        QueryMatchPrivate(AbstractRunner *r)
            : QSharedData(),
              runner(r),
              type(QueryMatch::ExactMatch),
              relevance(.7),
              selAction(0),
              enabled(true),
              idSetByData(false)
        {
        }

        QWeakPointer<AbstractRunner> runner;
        QueryMatch::Type type;
        QString id;
        QString text;
        QString subtext;
        QIcon icon;
        QVariant data;
        qreal relevance;
        QAction *selAction;
        bool enabled : 1;
        bool idSetByData : 1;
};

QueryMatch::QueryMatch(AbstractRunner *runner)
    : d(new QueryMatchPrivate(runner))
{
//    kDebug() << "new match created";
}

QueryMatch::QueryMatch(const QueryMatch &other)
    : d(other.d)
{
}

QueryMatch::~QueryMatch()
{
}

bool QueryMatch::isValid() const
{
    return d->runner != 0;
}

QString QueryMatch::id() const
{
    if (d->id.isEmpty() && d->runner) {
        return d->runner.data()->id();
    }

    return d->id;
}

void QueryMatch::setType(Type type)
{
    d->type = type;
}

QueryMatch::Type QueryMatch::type() const
{
    return d->type;
}

void QueryMatch::setRelevance(qreal relevance)
{
    d->relevance = qMax(qreal(0.0), relevance);
}

qreal QueryMatch::relevance() const
{
    return d->relevance;
}

AbstractRunner* QueryMatch::runner() const
{
    return d->runner.data();
}

void QueryMatch::setText(const QString &text)
{
    d->text = text;
}

void QueryMatch::setSubtext(const QString &subtext)
{
    d->subtext = subtext;
}

void QueryMatch::setData(const QVariant & data)
{
    d->data = data;

    if (d->id.isEmpty() || d->idSetByData) {
        const QString id = data.toString();
        if (!id.isEmpty()) {
            setId(data.toString());
            d->idSetByData = true;
        }
    }
}

void QueryMatch::setId(const QString &id)
{
    if (d->runner) {
        d->id = d->runner.data()->id();
    }

    if (!id.isEmpty()) {
        d->id.append('_').append(id);
    }

    d->idSetByData = false;
}

void QueryMatch::setIcon(const QIcon &icon)
{
    d->icon = icon;
}

QVariant QueryMatch::data() const
{
    return d->data;
}

QString QueryMatch::text() const
{
    return d->text;
}

QString QueryMatch::subtext() const
{
    return d->subtext;
}

QIcon QueryMatch::icon() const
{
    return d->icon;
}

void QueryMatch::setEnabled(bool enabled)
{
    d->enabled = enabled;
}

bool QueryMatch::isEnabled() const
{
    return d->enabled && d->runner;
}

QAction* QueryMatch::selectedAction() const
{
    return d->selAction;
}

void QueryMatch::setSelectedAction(QAction *action)
{
    d->selAction = action;
}

bool QueryMatch::operator<(const QueryMatch &other) const
{
    if (d->type == other.d->type) {
        if (isEnabled() != other.isEnabled()) {
            return other.isEnabled();
        }

        if (d->relevance != other.d->relevance) {
            return d->relevance < other.d->relevance;
        }

        // when resorting to sort by alpha, we want the
        // reverse sort order!
        return d->text > other.d->text;
    }

    return d->type < other.d->type;
}

QueryMatch &QueryMatch::operator=(const QueryMatch &other)
{
    if (d != other.d) {
        d = other.d;
    }

    return *this;
}

bool QueryMatch::operator==(const QueryMatch &other) const
{
    return (d == other.d);
}

void QueryMatch::run(const RunnerContext &context) const
{
    //kDebug() << "we run the term" << context->query() << "whose type is" << context->mimetype();
    if (d->runner) {
        d->runner.data()->run(context, *this);
    }
}

bool QueryMatch::hasConfigurationInterface() const
{
    return d->runner && d->runner.data()->hasRunOptions();
}

void QueryMatch::createConfigurationInterface(QWidget *parent)
{
    if (hasConfigurationInterface()) {
        d->runner.data()->createRunOptions(parent);
    }
}

} // Plasma namespace

