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
#include <QSharedData>

#include <KCompletion>
#include <KDebug>
#include <KMimeType>
#include <KShell>
#include <KStandardDirs>
#include <KUrl>

#include "searchmatch.h"

namespace Plasma
{

class SearchContext::Private : public QSharedData
{
    public:
        Private(SearchContext *context, SearchContext::DataPolicy p)
            : QSharedData(),
              type(SearchContext::UnknownType),
              q(context),
              policy(p)
        {
        }

        Private(const SearchContext::Private& p)
            : QSharedData(),
              term(p.term),
              mimeType(p.mimeType),
              type(p.type),
              q(p.q),
              policy(p.policy)
        {
            //kDebug() << "¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿boo yeah";
        }

        ~Private()
        {
            lockForWrite();
            qDeleteAll(matches);
            matches.clear();
            unlock();
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

        /**
         * Determines type of query
         */
        void determineType()
        {
            lockForWrite();
            QString path = KShell::tildeExpand(term);
        
            int space = term.indexOf(' ');
            if (space > 0) {
                if (!KStandardDirs::findExe(path.left(space)).isEmpty()) {
                    type = ShellCommand;
                }
            } else if (!KStandardDirs::findExe(path.left(space)).isEmpty()) {
                type = Executable;
            } else {
                KUrl url(term);
                if (!url.protocol().isEmpty() && !url.host().isEmpty()) {
                    type = NetworkLocation;
                } else  if (QFile::exists(path)) {
                    QFileInfo info(path);
                    if (info.isDir()) {
                        type = Directory;
                        mimeType = "inode/folder";
                    } else {
                        type = File;
                        KMimeType::Ptr mimeTypePtr = KMimeType::findByPath(path);
                        if (mimeTypePtr) {
                            mimeType = mimeTypePtr->name();
                        }
                    }
                } else if (term.contains('.')) {
                    // default to a network location so we can can do things like www.kde.org
                    type = NetworkLocation;
                }
            }
            unlock();
        }
        
        QReadWriteLock lock;
        QList<SearchMatch*> matches;
        QString term;
        QString mimeType;
        SearchContext::Type type;
        SearchContext * q;
        const SearchContext::DataPolicy policy;
};


SearchContext::SearchContext(QObject *parent, DataPolicy policy)
    : QObject(parent),
      d(new Private(this, policy))
{
}

//copy ctor
SearchContext::SearchContext(SearchContext &other, QObject *parent)
     : QObject(parent)
{
    other.d->lockForRead();
    d=other.d;
    other.d->unlock();
}

SearchContext::~SearchContext()
{
}

void SearchContext::reset()
{
    // Locks are needed as other contexts can be copied of this one

    // We will detach if we are a copy of someone. But we will reset 
    // if we are the 'main' context others copied from. Resetting 
    // one SearchContext makes all the copies oneobsolete.  
    d.detach();

    //kDebug() << "reset searchContext";
    d->lockForWrite();
    d->type = SearchContext::UnknownType;
    d->term.clear();
    d->mimeType.clear();
    d->unlock();

    // we still have to remove all the matches, since if the
    // ref count was 1 (e.g. only the SearchContext is using
    // the dptr) then we won't get a copy made
    removeAllMatches();

    //kDebug() << "match count" << d->matches.count();
}

void SearchContext::setSearchTerm(const QString &term)
{
    reset();

    if (term.isEmpty()) {
        return;
    }

    d->lockForWrite();
    d->term = term;
    d->unlock();
    d->determineType();
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

bool SearchContext::addMatches(const QString& term, const QList<SearchMatch*> &matches)
{
    if (searchTerm() != term || matches.isEmpty()) {
        return false;
    }

    d->lockForWrite();
    d->matches << matches;
    d->unlock();
    //kDebug()<< "add matches";
    // A copied searchContext may share the d pointer, 
    // we always want to sent the signal of the object that created
    // the d pointer 
    emit d->q->matchesChanged();
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
    //kDebug()<< "added match" << match->text();
    emit d->q->matchesChanged();

    return true;
}


QList<SearchMatch *> SearchContext::matches() const
{
    d->lockForRead();
    QList<SearchMatch *> matches = d->matches;
    d->unlock();
    return matches;
}

void SearchContext::removeAllMatches()
{
    d->lockForWrite();
    if (!d->matches.isEmpty()) {
        d->matches.clear();
        d->unlock();
        emit d->q->matchesChanged();
        // in case someone is still holding on to the Matches
        // when we emit the matchesChanged() signal, we don't
        // delete the matches until after the signal is handled.
        // a bit safer.
       d->lockForWrite();
       qDeleteAll(d->matches);
    }
    d->unlock();
}

}

#include "searchcontext.moc"
