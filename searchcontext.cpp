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

#include <QFile>
#include <QFileInfo>

#include <KCompletion>
#include <KDebug>
#include <KMimeType>
#include <KStandardDirs>
#include <KUrl>

#include "searchaction.h"

namespace Plasma
{

class SearchContext::Private
{
    public:
        Private()
            : type(SearchContext::UnknownType),
              completer(0)
        {
        }

        ~Private()
        {
            delete completer;
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

            if (completer) {
                completer->clear();
            }
        }

        KCompletion* completionObject()
        {
            if (!completer) {
                completer = new KCompletion;
            }

            return completer;
        }

        QList<SearchAction *> info;
        QList<SearchAction *> exact;
        QList<SearchAction *> possible;
        QString term;
        QString mimetype;
        SearchContext::Type type;
        KCompletion *completer;
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

void SearchContext::setSearchTerm(const QString &term)
{
    d->resetState();

    if (term.isEmpty()) {
        return;
    }

    d->term = term;

    int space = term.indexOf(' ');
    if (space > 0) {
        if (!KStandardDirs::findExe(term.left(space)).isEmpty()) {
            d->type = ShellCommand;
        }
    } else if (!KStandardDirs::findExe(term.left(space)).isEmpty()) {
        d->type = Executable;
    } else {
        KUrl url(term);

        if (!url.protocol().isEmpty() && !url.host().isEmpty()) {
            d->type = NetworkLocation;
        } else  if (QFile::exists(term)) {
            QFileInfo info(term);
            if (info.isDir()) {
                d->type = Directory;
                d->mimetype = "inode/folder";
            } else {
                d->type = File;
                KMimeType::Ptr mimetype = KMimeType::findByPath(term);
                if (mimetype) {
                    d->mimetype = mimetype->name();
                }
            }
        } else if (term.contains('.')) {
            // default to a network location so we can can do things like www.kde.org
            d->type = NetworkLocation;
        }
    }
}

QString SearchContext::searchTerm() const
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

KCompletion* SearchContext::completionObject() const
{
    return d->completionObject();
}

void SearchContext::addStringCompletion(const QString &completion)
{
    if (!d->completer) {
        // if the completion object isn't actually used, don't bother
        return;
    }

    d->completer->addItem(completion);
}

void SearchContext::addStringCompletions(const QStringList &completion)
{
    if (!d->completer) {
        // if the completion object isn't actually used, don't bother
        return;
    }

    d->completer->insertItems(completion);
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

}

#include "searchcontext.moc"
