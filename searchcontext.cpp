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
            qDeleteAll(matches);
            matches.clear();
            delete completer;
        }

        void resetState()
        {
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
        QList<SearchMatch *> matches;
        QString term;
        QString mimeType;
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
    d->mimeType = other.d->mimeType;
    d->type = other.d->type;
    other.d->unlock();
}

SearchContext::~SearchContext()
{
    delete d;
}

void SearchContext::resetSearchTerm(const QString &term)
{
    d->lockForWrite();
    QList<SearchMatch*> matches = d->matches;

    d->matches.clear();
    d->type = SearchContext::UnknownType;
    d->term.clear();
    d->mimeType.clear();

    if (d->completer) {
        d->completer->clear();
    }

    d->unlock();

    emit matchesChanged();

    // in case someone is still holding on to the Matches
    // when we emit the matchesChanged() signal, we don't
    // delete the matches until after the signal is handled.
    // a bit safer.
    qDeleteAll(matches);
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
                d->mimeType = "inode/folder";
            } else {
                d->type = File;
                KMimeType::Ptr mimeType = KMimeType::findByPath(path);
                if (mimeType) {
                    d->mimeType = mimeType->name();
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

QString SearchContext::mimeType() const
{
    return d->mimeType;
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

bool SearchContext::addMatches(const QString& term, const QList<SearchMatch*> &matches)
{
    if (searchTerm() != term || matches.isEmpty()) {
        return false;
    }

    d->lockForWrite();
    d->matches << matches;
    d->unlock();

    // TODO: perhaps queue this signal so that it is only emitted after a small delay?
    //       currently we do this in krunner's Interface class, but it would probably
    //       be better to move that detail to SearchContext so that other apps that
    //       use SearchContext can also benefit from it
    emit matchesChanged();
    return true;
}

bool SearchContext::addMatch(const QString &term, SearchMatch *match)
{
    if (searchTerm() != term) {
        return false;
    }

    d->lockForWrite();
    d->matches << match;
    d->unlock();
    emit matchesChanged();

    return true;
}

bool SearchContext::addMatchesTo(SearchContext &other)
{
    //NOTE: we have addMatchesTo instead of the more 'natural' addMatches
    // because we can get away with one write lock on the local object
    // this way, otherwise we'd need to lock once for searchTerm, once
    // for matches() and again for clearMatches() (2 read, one write)
    d->lockForWrite();

    const bool success = other.addMatches(d->term, d->matches);

    if (success) {
        // the matches no longer belong to this SearchContext,
        // so remove them from the data
        d->matches.clear();
    }

    d->unlock();
    return success;
}

QList<SearchMatch *> SearchContext::matches() const
{
    d->lockForRead();
    QList<SearchMatch *> matches = d->matches;
    d->unlock();
    return matches;
}

void SearchContext::clearMatches()
{
    d->lockForWrite();

    QList<SearchMatch*> matches = d->matches;
    d->matches.clear();
    d->unlock();

    emit matchesChanged();

    // in case someone is still holding on to the Matches
    // when we emit the matchesChanged() signal, we don't
    // delete the matches until after the signal is handled.
    // a bit safer.
    qDeleteAll(matches);
}

}

#include "searchcontext.moc"
