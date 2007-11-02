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

#include <KCompletion>
#include <KDebug>
#include <KMimeType>
#include <KUriFilterData>

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
                //kDebug() << "term is a network protocol?" << term << filter.uriType();
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
