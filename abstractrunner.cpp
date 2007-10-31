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

#include "abstractrunner.h"

#include <KDebug>
#include <KMimeType>
#include <KServiceTypeTrader>
#include <KUriFilterData>


namespace Plasma
{

class SearchContext::Private
{
    public:
        Private()
            : type(SearchContext::UnknownType)
        {
        }

        void resetState()
        {
            qDeleteAll(info);
            info.clear();
            qDeleteAll(exact);
            exact.clear();
            qDeleteAll(possible);
            possible.clear();
            type = SearchContext::UnknownType;
            term.clear();
            mimetype.clear();
        }

        QList<SearchAction *> info;
        QList<SearchAction *> exact;
        QList<SearchAction *> possible;
        QString term;
        QString mimetype;
        SearchContext::Type type;
};

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

class AbstractRunner::Private
{
    public:
        bool hasMatchOptions;
        bool hasConfig;
};

SearchContext::SearchContext(QObject *parent)
    : QObject(parent),
      d(new Private)
{
}

SearchContext::~SearchContext()
{
    delete d;
}

void SearchContext::setTerm(const QString &term)
{
    d->resetState();

    if (term.isEmpty()) {
        return;
    }

    d->term = term;
    //FIXME: this is insanely slow =/
    KUriFilterData filter(term);
    bool filtered = KUriFilter::self()->filterUri(filter);

    if (filtered) {
        switch (filter.uriType()) {
            case KUriFilterData::LocalDir:
                d->type = Directory;
                d->mimetype = "inode/folder";
                break;
            case KUriFilterData::LocalFile: {
                d->type = File;
                KMimeType::Ptr mimetype = KMimeType::findByPath(filter.uri().path());
                if (mimetype) {
                    d->mimetype = mimetype->name();
                }
                break;
            }
            case KUriFilterData::NetProtocol:
                kDebug() << "term is a network protocol?" << term << filter.uriType();
                d->type = NetworkLocation;
                break;
            case KUriFilterData::Executable:
                d->type = Executable;
                break;
            case KUriFilterData::Shell:
                d->type = ShellCommand;
                break;
            case KUriFilterData::Help:
                d->type = Help;
                break;
            default:
                break;
        }
    }
}

QString SearchContext::term() const
{
    return d->term;
}

SearchContext::Type SearchContext::type() const
{
    return d->type;
}

QString SearchContext::mimetype() const
{
    return d->mimetype;
}

SearchAction* SearchContext::addInformationalMatch(AbstractRunner *runner)
{
    SearchAction *action = new SearchAction(this, runner);
    action->setType(SearchAction::InformationalMatch);
    d->info.append(action);
    return action;
}

SearchAction* SearchContext::addExactMatch(AbstractRunner *runner)
{
    SearchAction *action = new SearchAction(this, runner);
    action->setType(SearchAction::ExactMatch);
    d->exact.append(action);
    return action;
}

SearchAction* SearchContext::addPossibleMatch(AbstractRunner *runner)
{
    SearchAction *action = new SearchAction(this, runner);
    action->setType(SearchAction::PossibleMatch);
    d->possible.append(action);
    return action;
}

QList<SearchAction *> SearchContext::informationalMatches() const
{
    return d->info;
}

QList<SearchAction *> SearchContext::exactMatches() const
{
    return d->exact;
}

QList<SearchAction *> SearchContext::possibleMatches() const
{
    return d->possible;
}



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

QString SearchAction::term() const
{
    return d->search->term();
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


AbstractRunner::AbstractRunner(QObject* parent)
    : QObject(parent),
      d(new Private())
{
}

AbstractRunner::~AbstractRunner()
{
    delete d;
}

bool AbstractRunner::hasMatchOptions()
{
    return d->hasMatchOptions;
}

void AbstractRunner::setHasMatchOptions(bool hasMatchOptions)
{
    d->hasMatchOptions = hasMatchOptions;
}

void AbstractRunner::createMatchOptions(QWidget *parent)
{
    Q_UNUSED(parent)
}

bool AbstractRunner::canBeConfigured()
{
    return d->hasConfig;
}

void AbstractRunner::setCanBeConfigured(bool hasConfig)
{
    d->hasConfig = hasConfig;
}

void AbstractRunner::createConfigurationInterface(QWidget *widget)
{
    Q_UNUSED(widget)
}

void AbstractRunner::exec(Plasma::SearchAction *action)
{
    Q_UNUSED(action)
}

AbstractRunner::List AbstractRunner::loadRunners(QObject* parent)
{
    List firstRunners;
    List runners;
    List lastRunners;

    KService::List offers = KServiceTypeTrader::self()->query("Plasma/Runner");
    QString error;
    foreach (KService::Ptr service, offers) {
        AbstractRunner* runner = service->createInstance<AbstractRunner>(parent, QVariantList(), &error);
        if (runner) {
            //kDebug() << "loaded runner : " << service->name();
            QString phase = service->property("X-Plasma-RunnerPhase").toString();
            if (phase == "last") {
                lastRunners.append(runner);
            } else if (phase == "first") {
                firstRunners.append(runner);
            } else {
                runners.append(runner);
            }
        }
        else {
            kDebug() << "failed to load runner : " << service->name() << ". error reported: " << error;
        }
    }

    firstRunners << runners << lastRunners;
    return firstRunners;
}

} // Plasma namespace

#include "abstractrunner.moc"
