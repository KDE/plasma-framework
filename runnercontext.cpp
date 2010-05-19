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

#include <cmath>

#include <QReadWriteLock>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QSharedData>

#include <kcompletion.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kmimetype.h>
#include <kshell.h>
#include <kstandarddirs.h>
#include <kurl.h>
#include <kprotocolinfo.h>

#include "abstractrunner.h"
#include "querymatch.h"

//#define LOCK_FOR_READ(d) if (d->policy == Shared) { d->lock.lockForRead(); }
//#define LOCK_FOR_WRITE(d) if (d->policy == Shared) { d->lock.lockForWrite(); }
//#define UNLOCK(d) if (d->policy == Shared) { d->lock.unlock(); }

#define LOCK_FOR_READ(d) d->lock.lockForRead();
#define LOCK_FOR_WRITE(d) d->lock.lockForWrite();
#define UNLOCK(d) d->lock.unlock();

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
bool correctPathCase(const QString& path, QString &corrected)
{
    // early exit check
    if (QFile::exists(path)) {
        corrected = path;
        return true;
    }

    // path components
    QStringList components = QString(path).split(QDir::separator());

    if (components.size() < 1) {
        return false;
    }

    const bool mustBeDir = components.back().isEmpty();

    //kDebug() << "Components are" << components;

    if (mustBeDir) {
        components.pop_back();
    }

    if (components.isEmpty()) {
        return true;
    }

    QString correctPath;
    const unsigned initialComponents = components.size();
    for (unsigned i = 0; i < initialComponents - 1; i ++) {
        const QString tmp = components[0] + QDir::separator() + components[1];

        if (!correctLastComponentCase(tmp, correctPath, components.size() > 2 || mustBeDir)) {
            //kDebug() << "search was not successful";
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
              q(context),
              singleRunnerQueryMode(false)
        {
        }

        RunnerContextPrivate(const RunnerContextPrivate &p)
            : QSharedData(),
              launchCounts(p.launchCounts),
              type(RunnerContext::None),
              q(p.q),
              singleRunnerQueryMode(false)
        {
            //kDebug() << "¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿¿boo yeah" << type;
        }

        ~RunnerContextPrivate()
        {
        }

        /**
         * Determines type of query
                    &&
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
                // check for a normal URL first
                //kDebug() << url << KProtocolInfo::protocolClass(url.protocol()) << url.hasHost() <<
                //    url.host() << url.isLocalFile() << path << path.indexOf('/');
                const bool hasProtocol = !url.protocol().isEmpty();
                const bool isLocalProtocol = KProtocolInfo::protocolClass(url.protocol()) == ":local";
                if (hasProtocol && 
                    ((!isLocalProtocol && url.hasHost()) ||
                     (isLocalProtocol && url.protocol() != "file"))) {
                    // we either have a network protocol with a host, so we can show matches for it
                    // or we have a non-file url that may be local so a host isn't required
                    type = RunnerContext::NetworkLocation;
                } else if (isLocalProtocol) {
                    // at this point in the game, we assume we have a path,
                    // but if a path doesn't have any slashes
                    // it's too ambiguous to be sure we're in a filesystem context
                    path = QDir::cleanPath(url.toLocalFile());
                    //kDebug( )<< "slash check" << path;
                    if (hasProtocol || ((path.indexOf('/') != -1 || path.indexOf('\\') != -1))) {
                        QString correctCasePath;
                        if (correctPathCase(path, correctCasePath)) {
                            path = correctCasePath;
                            QFileInfo info(path);
                            //kDebug( )<< "correct cas epath is" << correctCasePath << info.isSymLink() <<
                            //    info.isDir() << info.isFile();

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
            }

            //kDebug() << "term2type" << term << type;
        }

        void invalidate()
        {
            q = &s_dummyContext;
        }

        QReadWriteLock lock;
        QList<QueryMatch> matches;
        QMap<QString, const QueryMatch*> matchesById;
        QHash<QString, int> launchCounts;
        QString term;
        QString mimeType;
        RunnerContext::Type type;
        RunnerContext * q;
        static RunnerContext s_dummyContext;
        bool singleRunnerQueryMode;
};

RunnerContext RunnerContextPrivate::s_dummyContext;

RunnerContext::RunnerContext(QObject *parent)
    : QObject(parent),
      d(new RunnerContextPrivate(this))
{
}

//copy ctor
RunnerContext::RunnerContext(RunnerContext &other, QObject *parent)
    : QObject(parent)
{
    LOCK_FOR_READ(other.d)
    d = other.d;
    UNLOCK(other.d)
}

RunnerContext::~RunnerContext()
{
}

RunnerContext &RunnerContext::operator=(const RunnerContext &other)
{
    if (this->d == other.d) {
        return *this;
    }

    QExplicitlySharedDataPointer<Plasma::RunnerContextPrivate> oldD = d;
    LOCK_FOR_WRITE(d)
    LOCK_FOR_READ(other.d)
    d = other.d;
    UNLOCK(other.d)
    UNLOCK(oldD)
    return *this;
}

void RunnerContext::reset()
{
    // We will detach if we are a copy of someone. But we will reset
    // if we are the 'main' context others copied from. Resetting
    // one RunnerContext makes all the copies obsolete.

    // We need to mark the q pointer of the detached RunnerContextPrivate
    // as dirty on detach to avoid receiving results for old queries
    d->invalidate();

    d.detach();

    // Now that we detached the d pointer we need to reset its q pointer

    d->q = this;

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
    d->singleRunnerQueryMode = false;
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

bool RunnerContext::isValid() const
{
    // if our qptr is dirty, we aren't useful anymore
    return (d->q != &(d->s_dummyContext));
}

bool RunnerContext::addMatches(const QString &term, const QList<QueryMatch> &matches)
{
    Q_UNUSED(term)

    if (matches.isEmpty() || !isValid()) {
       //Bail out if the query is empty or the qptr is dirty
        return false;
    }

    LOCK_FOR_WRITE(d)
    foreach (QueryMatch match, matches) {
        // Give previously launched matches a slight boost in relevance
        // The boost smoothly saturates to 0.5;
        if (int count = d->launchCounts.value(match.id())) {
            match.setRelevance(match.relevance() + 0.5 * (1-exp(-count*0.3)));
        }

        d->matches.append(match);
#ifndef NDEBUG
        if (d->matchesById.contains(match.id())) {
            kDebug() << "Duplicate match id " << match.id() << "from" << match.runner()->name();
        }
#endif
        d->matchesById.insert(match.id(), &d->matches.at(d->matches.size() - 1));
    }
    UNLOCK(d);
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

    if (!isValid()) {
        // Bail out if the qptr is dirty
        return false;
    }

    QueryMatch m(match); // match must be non-const to modify relevance

    LOCK_FOR_WRITE(d)

    if (int count = d->launchCounts.value(m.id())) {
        m.setRelevance(m.relevance() + 0.05 * count);
    }

    d->matches.append(m);
    d->matchesById.insert(m.id(), &d->matches.at(d->matches.size() - 1));
    UNLOCK(d);
    //kDebug()<< "added match" << match->text();
    emit d->q->matchesChanged();

    return true;
}

bool RunnerContext::removeMatches(const QStringList matchIdList)
{
    if (!isValid()) {
        return false;
    }

    QStringList presentMatchIdList;
    QList<const QueryMatch*> presentMatchList;

    LOCK_FOR_READ(d)
    foreach(const QString &matchId, matchIdList) {
        const QueryMatch* match = d->matchesById.value(matchId, 0);
        if (match) {
            presentMatchList << match;
            presentMatchIdList << matchId;
        }
    }
    UNLOCK(d)

    if (presentMatchIdList.isEmpty()) {
        return false;
    }

    LOCK_FOR_WRITE(d)
    foreach(const QueryMatch *match, presentMatchList) {
        d->matches.removeAll(*match);
    }
    foreach(const QString &matchId, presentMatchIdList) {
        d->matchesById.remove(matchId);
    }
    UNLOCK(d)

    emit d->q->matchesChanged();

    return true;
}

bool RunnerContext::removeMatch(const QString matchId)
{
    if (!isValid()) {
        return false;
    }
    LOCK_FOR_READ(d)
    const QueryMatch* match = d->matchesById.value(matchId, 0);
    UNLOCK(d)
    if (!match) {
        return false;
    }
    LOCK_FOR_WRITE(d)
    d->matches.removeAll(*match);
    d->matchesById.remove(matchId);
    UNLOCK(d)
    emit d->q->matchesChanged();

    return true;
}

QList<QueryMatch> RunnerContext::matches() const
{
    LOCK_FOR_READ(d)
    QList<QueryMatch> matches = d->matches;
    UNLOCK(d);
    return matches;
}

QueryMatch RunnerContext::match(const QString &id) const
{
    LOCK_FOR_READ(d)
    const QueryMatch *match = d->matchesById.value(id, 0);
    UNLOCK(d)

    if (match) {
        return *match;
    }

    return QueryMatch(0);
}

void RunnerContext::setSingleRunnerQueryMode(bool enabled)
{
    d->singleRunnerQueryMode = enabled;
}

bool RunnerContext::singleRunnerQueryMode() const
{
    return d->singleRunnerQueryMode;
}

void RunnerContext::restore(const KConfigGroup &config)
{
    const QStringList cfgList = config.readEntry("LaunchCounts", QStringList());

    const QRegExp r("(\\d*) (.*)");
    foreach (const QString& entry, cfgList) {
        r.indexIn(entry);
        int count = r.cap(1).toInt();
        QString id = r.cap(2);
        d->launchCounts[id] = count;
    }
}

void RunnerContext::save(KConfigGroup &config)
{
    QStringList countList;

    typedef QHash<QString, int>::const_iterator Iterator;
    Iterator end = d->launchCounts.constEnd();
    for (Iterator i = d->launchCounts.constBegin(); i != end; ++i) {
        countList << QString("%2 %1").arg(i.key()).arg(i.value());
    }

    config.writeEntry("LaunchCounts", countList);
    config.sync();
}

void RunnerContext::run(const QueryMatch &match)
{
    ++d->launchCounts[match.id()];
    match.run(*this);
}

} // Plasma namespace

#include "runnercontext.moc"
