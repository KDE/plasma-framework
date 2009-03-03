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

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSharedData>

#include <kcompletion.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <kurl.h>

#include "abstractrunner.h"
#include "querymatch.h"

//#define LOCK_FOR_READ(context) if (context->d->policy == Shared) { context->d->lock.lockForRead(); }
//#define LOCK_FOR_WRITE(context) if (context->d->policy == Shared) { context->d->lock.lockForWrite(); }
//#define UNLOCK(context) if (context->d->policy == Shared) { context->d->lock.unlock(); }

#define LOCK_FOR_READ(context) context->d->lock.lockForRead();
#define LOCK_FOR_WRITE(context) context->d->lock.lockForWrite();
#define UNLOCK(context) context->d->lock.unlock();

namespace Plasma
{

/*
Corrects the case of the last component in a path (e.g. /usr/liB -> /usr/lib)
path: The path to be processed.
correctCasePath: The corrected-case path
mustBeDir: Tells whether the last component is a folder or doesn't matter
Returns true on success and false on error, in case of error, correctCasePath is not modified
*/
bool correctLastComponentCase(const QString &path, QString &correctCasePath, const bool mustBeDir)
{
    //kDebug() << "Correcting " << path;

    // If the file already exists then no need to search for it.
    if (QFile::exists(path)) {
        correctCasePath = path;
        //kDebug() << "Correct path is" << correctCasePath;
        return true;
    }

    const QFileInfo pathInfo(path);

    const QDir fileDir = pathInfo.dir();
    //kDebug() << "Directory is" << fileDir;

    const QString filename = pathInfo.fileName();
    //kDebug() << "Filename is" << filename;

    //kDebug() << "searching for a" << (mustBeDir ? "directory" : "directory/file");

    const QStringList matchingFilenames = fileDir.entryList(QStringList(filename),
                                          mustBeDir ? QDir::Dirs : QDir::NoFilter);

    if (matchingFilenames.empty()) {
        //kDebug() << "No matches found!!\n";
        return false;
    } else {
        /*if (matchingFilenames.size() > 1) {
            kDebug() << "Found multiple matches!!\n";
        }*/

        if (fileDir.path().endsWith(QDir::separator())) {
            correctCasePath = fileDir.path() + matchingFilenames[0];
        } else {
            correctCasePath = fileDir.path() + QDir::separator() + matchingFilenames[0];
        }

        //kDebug() << "Correct path is" << correctCasePath;
        return true;
    }
}

/*
Corrects the case of a path (e.g. /uSr/loCAL/bIN -> /usr/local/bin)
path: The path to be processed.
corrected: The corrected-case path
Returns true on success and false on error, in case of error, corrected is not modified
*/
bool correctPathCase(const QString path, QString &corrected)
{
    // early exit check
    if (QFile::exists(path)) {
        corrected = path;
        return true;
    }

    // path components
    QStringList components = QString(path).split(QDir::separator());

    if (components.size() < 2) {
        return false;
    }

    const bool mustBeDir = components.back().isEmpty();

    //kDebug() << "Components are" << components;

    QString correctPath;

    if (components.back().isEmpty()) {
        components.pop_back();
    }

    const unsigned initialComponents = components.size();
    for (unsigned i = 0; i < initialComponents - 1; i ++) {
        const QString tmp = components[0] + QDir::separator() + components[1];

        if (!correctLastComponentCase(tmp, correctPath, components.size() > 2 || mustBeDir)) {
            //kDebug() << "search was not successfull";
            return false;
        }

        components.removeFirst();
        components[0] = correctPath;
    }

    corrected = correctPath;
    return true;
}

class RunnerContextPrivate : public QSharedData
{
    public:
        RunnerContextPrivate(RunnerContext *context)
            : QSharedData(),
              type(RunnerContext::UnknownType),
              q(context)
        {
        }

        RunnerContextPrivate(const RunnerContextPrivate &p)
            : QSharedData(),
              type(RunnerContext::None),
              q(p.q)
        {
            //kDebug() << "¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿boo yeah" << type;
        }

        ~RunnerContextPrivate()
        {
        }

        /**
         * Determines type of query
         */
        void determineType()
        {
            // NOTE! this method must NEVER be called from
            // code that may be running in multiple threads
            // with the same data.
            type = RunnerContext::UnknownType;
            QString path = QDir::cleanPath(KShell::tildeExpand(term));

            int space = path.indexOf(' ');
            if (!KStandardDirs::findExe(path.left(space)).isEmpty()) {
                // it's a shell command if there's a space because that implies
                // that it has arguments!
                type = (space > 0) ? RunnerContext::ShellCommand :
                                     RunnerContext::Executable;
            } else {
                KUrl url(term);
                QString correctCasePath;
                if (!url.protocol().isEmpty() && !url.isLocalFile()) {
                    type = RunnerContext::NetworkLocation;
                } else if (correctPathCase(path, correctCasePath)) {
                    path = correctCasePath;
                    QFileInfo info(path);
                    if (info.isSymLink()) {
                        path = info.canonicalFilePath();
                        info = QFileInfo(path);
                    }
                    if (info.isDir()) {
                        type = RunnerContext::Directory;
                        mimeType = "inode/folder";
                    } else if (info.isFile()) {
                        type = RunnerContext::File;
                        KMimeType::Ptr mimeTypePtr = KMimeType::findByPath(path);
                        if (mimeTypePtr) {
                            mimeType = mimeTypePtr->name();
                        }
                    }
                }
            }
        }

        QReadWriteLock lock;
        QList<QueryMatch> matches;
        QMap<QString, const QueryMatch*> matchesById;
        QString term;
        QString mimeType;
        RunnerContext::Type type;
        RunnerContext * q;
};

RunnerContext::RunnerContext(QObject *parent)
    : QObject(parent),
      d(new RunnerContextPrivate(this))
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
    // We will detach if we are a copy of someone. But we will reset
    // if we are the 'main' context others copied from. Resetting
    // one RunnerContext makes all the copies obsolete.
    
    d->q = 0; //we need to mark the q pointer of the detached RunnerContextPrivate 
              //as dirty on detach to avoid receiving results for old queries   

    d.detach();
    
    d->q = this; //now that we detached the d pointer we need to mark its q pointer as
                 //this to receive the signals 

    // we still have to remove all the matches, since if the
    // ref count was 1 (e.g. only the RunnerContext is using
    // the dptr) then we won't get a copy made
    if (!d->matches.isEmpty()) {
        d->matchesById.clear();
        d->matches.clear();
        emit matchesChanged();
    }

    d->term.clear();
    d->mimeType.clear();
    d->type = UnknownType;
    //kDebug() << "match count" << d->matches.count();
}

void RunnerContext::setQuery(const QString &term)
{
    reset();

    if (term.isEmpty()) {
        return;
    }

    d->term = term;
    d->determineType();
}

QString RunnerContext::query() const
{
    // the query term should never be set after
    // a search starts. in fact, reset() ensures this
    // and setQuery(QString) calls reset()
    return d->term;
}

RunnerContext::Type RunnerContext::type() const
{
    return d->type;
}

QString RunnerContext::mimeType() const
{
    return d->mimeType;
}

bool RunnerContext::addMatches(const QString &term, const QList<QueryMatch> &matches)
{
    Q_UNUSED(term)

    if (matches.isEmpty() || (!d->q)) { //Bail out if the query is empty or the qptr is dirty 
        return false;
    }

    LOCK_FOR_WRITE(this)
    foreach (const QueryMatch &match, matches) {
        d->matches.append(match);
#ifndef NDEBUG
        if (d->matchesById.contains(match.id())) {
                kDebug() << "Duplicate match id " << match.id() << "from" << match.runner()->name();
        }
#endif
        d->matchesById.insert(match.id(), &d->matches.at(d->matches.size() - 1));
    }
    UNLOCK(this);
    //kDebug()<< "add matches";
    // A copied searchContext may share the d pointer,
    // we always want to sent the signal of the object that created
    // the d pointer
    emit d->q->matchesChanged();
    
    return true;
}

bool RunnerContext::addMatch(const QString &term, const QueryMatch &match)
{
    Q_UNUSED(term)

    if (!d->q) { // Bail out if the qptr is dirty
        return false;
    }
    
    LOCK_FOR_WRITE(this)
    d->matches.append(match);
    d->matchesById.insert(match.id(), &d->matches.at(d->matches.size() - 1));
    UNLOCK(this);
    //kDebug()<< "added match" << match->text();
    emit d->q->matchesChanged(); 
    

    return true;
}

QList<QueryMatch> RunnerContext::matches() const
{
    LOCK_FOR_READ(this)
    QList<QueryMatch> matches = d->matches;
    UNLOCK(this);
    return matches;
}

QueryMatch RunnerContext::match(const QString &id) const
{
    LOCK_FOR_READ(this)
    const QueryMatch *match = d->matchesById.value(id, 0);
    UNLOCK(this)

    if (match) {
        return *match;
    }

    return QueryMatch(0);
}

} // Plasma namespace

#include "runnercontext.moc"
