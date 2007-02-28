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

#include <KActionCollection>

#include "runner.h"

class Runner::Private
{
    public:
        Private( Runner* runner )
        {
            actions = new KActionCollection( runner );
        }

        KActionCollection* actions;
};

Runner::Runner( QObject* parent )
    : QObject( parent )
{
    d = new Private( this );
}

Runner::~Runner()
{
    delete d;
}

bool Runner::hasOptions()
{
    return false;
}

QWidget* Runner::options()
{
    return 0;
}

KActionCollection* Runner::matches( const QString& term, int max, int offset )
{
    d->actions->clear();
    fillMatches( d->actions, term, max, offset );
    return d->actions;
}

void Runner::fillMatches( KActionCollection* matches,
                          const QString& term,
                          int max, int offset )
{
    Q_UNUSED( term );
    Q_UNUSED( max );
    Q_UNUSED( offset );
}

#include "runner.moc"
