/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
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

#include <QAction>
#include <KActionCollection>
#include <KServiceTypeTrader>

#include "abstractrunner.h"

namespace Plasma
{

class AbstractRunner::Private
{
    public:
        Private( AbstractRunner* runner ) :
            exactMatch( 0 ),
            actions( new KActionCollection( runner ) )
        {
        }

        QAction* exactMatch;
        KActionCollection* actions;
        // FIXME: it's a bit lame to keep a copy of the term in each runner
        QString term;
};

AbstractRunner::AbstractRunner( QObject* parent )
    : QObject( parent )
{
    d = new Private( this );
}

AbstractRunner::~AbstractRunner()
{
    delete d;
}

bool AbstractRunner::hasOptions()
{
    return false;
}

QWidget* AbstractRunner::options()
{
    return 0;
}

QAction* AbstractRunner::exactMatch( )
{
    return d->exactMatch;
}

QAction* AbstractRunner::exactMatch( const QString& term )
{
    delete d->exactMatch;
    d->term.clear();

    d->exactMatch = accepts( term );
    if ( d->exactMatch ) {
        d->term = term;
        connect( d->exactMatch, SIGNAL( triggered() ),
                 this, SLOT( runExactMatch() ) );
    }

    return d->exactMatch;
}

KActionCollection* AbstractRunner::matches( const QString& term, int max, int offset )
{
    d->actions->clear();
    fillMatches( d->actions, term, max, offset );
    return d->actions;
}

void AbstractRunner::fillMatches( KActionCollection* matches,
                          const QString& term,
                          int max, int offset )
{
    Q_UNUSED( matches );
    Q_UNUSED( term );
    Q_UNUSED( max );
    Q_UNUSED( offset );
}

void AbstractRunner::runExactMatch()
{
    exec( d->term );
}

AbstractRunner::List AbstractRunner::loadRunners( QWidget* parent )
{
    List runners;
    KService::List offers = KServiceTypeTrader::self()->query( "KRunner/Runner" );
    foreach ( KService::Ptr service, offers ) {
        AbstractRunner* runner = KService::createInstance<AbstractRunner>( service, parent );
        if ( runner ) {
            kDebug() << "loaded runner : " << service->name() << endl ;
            runners.append( runner );
        }
        else {
            kDebug() << "failed to load runner : " << service->name() << endl ;
        }
    }

    return runners;
}

} // Plasma namespace

#include "abstractrunner.moc"
