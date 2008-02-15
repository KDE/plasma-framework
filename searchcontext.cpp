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

#include "searchcontext.h"

#include <QReadWriteLock>

#include <QFile>
#include <QFileInfo>

#include <KCompletion>
#include <KDebug>
#include <KMimeType>
#include <KShell>
#include <KStandardDirs>
#include <KUrl>

#include "searchmatch.h"

namespace Plasma
{

class SearchContext::Private
{
    public:
        Private(SearchContext::DataPolicy p)
            : type(SearchContext::UnknownType),
              completer(0),
              policy(p)
        {
        }

        ~Private()
        {
            clearMatches();
            delete completer;
        }

        void resetState()
        {
            lockForWrite();
            qDeleteAll(info);
            info.clear();
            qDeleteAll(exact);
            exact.clear();
            qDeleteAll(possible);
            possible.clear();
            type = SearchContext::UnknownType;
            term.clear();
            mimetype.clear();

            if (completer) {
                completer->clear();
            }
            unlock();
        }

        void clearMatches()
        {
            lockForWrite();
            qDeleteAll(info);
            info.clear();
            qDeleteAll(exact);
            exact.clear();
            qDeleteAll(possible);
            possible.clear();
            unlock();
        }

        KCompletion* completionObject()
        {
            if (!completer) {
                completer = new KCompletion;
            }

            return completer;
        }

        void lockForRead()
        {
            if (policy == Shared) {
                lock.lockForRead();
            }
        }

        void lockForWrite()
        {
            if (policy == Shared) {
                lock.lockForWrite();
            }
        }

        void unlock()
        {
            if (policy == Shared) {
                lock.unlock();
            }
        }

        QReadWriteLock lock;
        QList<SearchMatch *> info;
        QList<SearchMatch *> exact;
        QList<SearchMatch *> possible;
        QString term;
        QString mimetype;
        SearchContext::Type type;
        KCompletion *completer;
        const SearchContext::DataPolicy policy;
};


SearchContext::SearchContext(QObject *parent, DataPolicy policy)
    : QObject(parent),
      d(new Private(policy))
{
}

SearchContext::SearchContext(QObject *parent, const SearchContext &other)
    : QObject(parent),
      d(new Private(SingleConsumer))
{
    other.d->lockForRead();
    d->term = other.d->term;
    d->mimetype = other.d->mimetype;
    d->type = other.d->type;
    other.d->unlock();
}

SearchContext::~SearchContext()
{
    delete d;
}

void SearchContext::resetSearchTerm(const QString &term)
{
    d->resetState();
    emit matchesChanged();
    setSearchTerm(term);
}

void SearchContext::setSearchTerm(const QString &term)
{
    if (term.isEmpty()) {
        return;
    }

    d->lockForWrite();
    d->term = term;
    d->unlock();
    determineType();
}

void SearchContext::determineType()
{
    d->lockForWrite();
    QString term = d->term;
    QString path = KShell::tildeExpand(term);

    int space = term.indexOf(' ');
    if (space > 0) {
        if (!KStandardDirs::findExe(path.left(space)).isEmpty()) {
            d->type = ShellCommand;
        }
    } else if (!KStandardDirs::findExe(path.left(space)).isEmpty()) {
        d->type = Executable;
    } else {
        KUrl url(term);

        if (!url.protocol().isEmpty() && !url.host().isEmpty()) {
            d->type = NetworkLocation;
        } else  if (QFile::exists(path)) {
            QFileInfo info(path);
            if (info.isDir()) {
                d->type = Directory;
                d->mimetype = "inode/folder";
            } else {
                d->type = File;
                KMimeType::Ptr mimetype = KMimeType::findByPath(path);
                if (mimetype) {
                    d->mimetype = mimetype->name();
                }
            }
        } else if (term.contains('.')) {
            // default to a network location so we can can do things like www.kde.org
            d->type = NetworkLocation;
        }
    }
    d->unlock();
}

QString SearchContext::searchTerm() const
{
    d->lockForRead();
    QString term = d->term;
    d->unlock();
    return term;
}

SearchContext::Type SearchContext::type() const
{
    return d->type;
}

QString SearchContext::mimetype() const
{
    return d->mimetype;
}

KCompletion* SearchContext::completionObject() const
{
    d->lockForRead();
    KCompletion* comp = d->completionObject();
    d->unlock();
    return comp;
}

void SearchContext::addStringCompletion(const QString &completion)
{
    d->lockForWrite();
    if (!d->completer) {
        d->unlock();
        // if the completion object isn't actually used, don't bother
        return;
    }

    d->completer->addItem(completion);
    d->unlock();
}

void SearchContext::addStringCompletions(const QStringList &completion)
{
    d->lockForWrite();
    if (!d->completer) {
        d->unlock();
        // if the completion object isn't actually used, don't bother
        return;
    }

    d->completer->insertItems(completion);
    d->unlock();
}

SearchMatch* SearchContext::addInformationalMatch(AbstractRunner *runner)
{
    Q_ASSERT(d->policy == SingleConsumer);
    SearchMatch *action = new SearchMatch(this, runner);
    action->setType(SearchMatch::InformationalMatch);
    d->info.append(action);
    return action;
}

SearchMatch* SearchContext::addExactMatch(AbstractRunner *runner)
{
    Q_ASSERT(d->policy == SingleConsumer);
    SearchMatch *action = new SearchMatch(this, runner);
    action->setType(SearchMatch::ExactMatch);
    d->exact.append(action);
    return action;
}

SearchMatch* SearchContext::addPossibleMatch(AbstractRunner *runner)
{
    Q_ASSERT(d->policy == SingleConsumer);
    SearchMatch *action = new SearchMatch(this, runner);
    action->setType(SearchMatch::PossibleMatch);
    d->possible.append(action);
    return action;
}

bool SearchContext::addMatches( const QString& term, const QList<SearchMatch *> &exactMatches,
                                                      const QList<SearchMatch *> &possibleMatches,
                                                      const QList<SearchMatch *> &informationalMatches )
{
    if (searchTerm() != term) {
        return false;
    }

    if (exactMatches.isEmpty() &&
        possibleMatches.isEmpty() &&
        informationalMatches.isEmpty()) {
        return false;
    }

    d->lockForWrite();
    d->exact << exactMatches;
    d->possible << possibleMatches;
    d->info << informationalMatches;
    d->unlock();
    emit matchesChanged();
    return true;
}

bool SearchContext::addMatchesTo(SearchContext &other)
{
    d->lockForRead();
    if (other.addMatches(d->term, d->exact, d->possible, d->info)) {
        // the matches no longer belong to this SearchContext,
        // so remove them from the data
        d->exact.clear();
        d->possible.clear();
        d->info.clear();
        d->unlock();
        return true;
    }

    d->unlock();
    return false;
}

QList<SearchMatch *> SearchContext::informationalMatches() const
{
    d->lockForRead();
    QList<SearchMatch *> matches = d->info;
    d->unlock();
    return matches;
}

QList<SearchMatch *> SearchContext::exactMatches() const
{
    d->lockForRead();
    QList<SearchMatch *> matches = d->exact;
    d->unlock();
    return matches;
}

QList<SearchMatch *> SearchContext::possibleMatches() const
{
    d->lockForRead();
    QList<SearchMatch *> matches = d->possible;
    d->unlock();
    return matches;
}

void SearchContext::clearMatches()
{
    d->clearMatches();
    emit matchesChanged();
}

}

#include "searchcontext.moc"
