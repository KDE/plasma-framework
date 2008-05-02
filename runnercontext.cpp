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

#include "runnercontext.h"

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

#define LOCK_FOR_READ(context) if (context->d->policy == Shared) { context->d->lock.lockForRead(); }
#define LOCK_FOR_WRITE(context) if (context->d->policy == Shared) { context->d->lock.lockForWrite(); }
#define UNLOCK(context) if (context->d->policy == Shared) { context->d->lock.unlock(); }
/*
#define LOCK_FOR_READ(context) context->d->lock.lockForRead();
#define LOCK_FOR_WRITE(context) context->d->lock.lockForWrite();
#define UNLOCK(context) context->d->lock.unlock();
*/
namespace Plasma
{

class RunnerContext::Private : public QSharedData
{
    public:
        Private(RunnerContext *context, RunnerContext::DataPolicy p)
            : QSharedData(),
              type(RunnerContext::UnknownType),
              q(context),
              policy(p)
        {
        }

        Private(const RunnerContext::Private& p)
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
            if (policy == Shared) {
                lock.lockForWrite();
            }
            qDeleteAll(matches);
            matches.clear();
            if (policy == Shared) {
                lock.unlock();
            }
        }

        /**
         * Determines type of query
         */
        void determineType()
        {
            if (policy == Shared) {
                lock.lockForWrite();
            }

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

            if (policy == Shared) {
                lock.unlock();
            }
        }

        QReadWriteLock lock;
        QList<SearchMatch*> matches;
        QString term;
        QString mimeType;
        RunnerContext::Type type;
        RunnerContext * q;
        const RunnerContext::DataPolicy policy;
};


RunnerContext::RunnerContext(QObject *parent, DataPolicy policy)
    : QObject(parent),
      d(new Private(this, policy))
{
}

//copy ctor
RunnerContext::RunnerContext(RunnerContext &other, QObject *parent)
     : QObject(parent)
{
    LOCK_FOR_READ((&other))
    d = other.d;
    UNLOCK((&other))
}

RunnerContext::~RunnerContext()
{
}

void RunnerContext::reset()
{
    // Locks are needed as other contexts can be copied of this one

    // We will detach if we are a copy of someone. But we will reset 
    // if we are the 'main' context others copied from. Resetting 
    // one RunnerContext makes all the copies oneobsolete.  
    d.detach();

    LOCK_FOR_WRITE(this)
    //kDebug() << "reset searchContext";
    d->type = RunnerContext::UnknownType;
    d->term.clear();
    d->mimeType.clear();
    UNLOCK(this);

    // we still have to remove all the matches, since if the
    // ref count was 1 (e.g. only the RunnerContext is using
    // the dptr) then we won't get a copy made
    removeAllMatches();

    //kDebug() << "match count" << d->matches.count();
}

void RunnerContext::setSearchTerm(const QString &term)
{
    reset();

    if (term.isEmpty()) {
        return;
    }

    LOCK_FOR_WRITE(this)
    d->term = term;
    UNLOCK(this);
    d->determineType();
}


QString RunnerContext::searchTerm() const
{
    LOCK_FOR_READ(this)
    QString term = d->term;
    UNLOCK(this);
    return term;
}

RunnerContext::Type RunnerContext::type() const
{
    return d->type;
}

QString RunnerContext::mimeType() const
{
    return d->mimeType;
}

bool RunnerContext::addMatches(const QString& term, const QList<SearchMatch*> &matches)
{
    if (searchTerm() != term || matches.isEmpty()) {
        return false;
    }

    LOCK_FOR_WRITE(this)
    d->matches << matches;
    UNLOCK(this);
    //kDebug()<< "add matches";
    // A copied searchContext may share the d pointer, 
    // we always want to sent the signal of the object that created
    // the d pointer 
    emit d->q->matchesChanged();
    return true;
}

bool RunnerContext::addMatch(const QString &term, SearchMatch *match)
{
    if (searchTerm() != term) {
        return false;
    }

    LOCK_FOR_WRITE(this)
    d->matches << match;
    UNLOCK(this);
    //kDebug()<< "added match" << match->text();
    emit d->q->matchesChanged();

    return true;
}


QList<SearchMatch *> RunnerContext::matches() const
{
    LOCK_FOR_READ(this)
    QList<SearchMatch *> matches = d->matches;
    UNLOCK(this);
    return matches;
}

void RunnerContext::removeAllMatches()
{
    LOCK_FOR_WRITE(this)
    if (!d->matches.isEmpty()) {
        QList<SearchMatch*> matches = d->matches;
        d->matches.clear();
        UNLOCK(this);

        // in case someone is still holding on to the Matches
        // when we emit the matchesChanged() signal, we don't
        // delete the matches until after the signal is handled.
        // a bit safer.
        emit d->q->matchesChanged();

        qDeleteAll(matches);
    } else {
        UNLOCK(this);
    }
}

}

#include "runnercontext.moc"
