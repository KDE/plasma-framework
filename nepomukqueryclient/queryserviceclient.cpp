/*
   Copyright (c) 2008 Sebastian Trueg <trueg@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "queryserviceclient.h"
#include "dbusoperators.h"
#include "result.h"
#include "query.h"
#include "queryserviceinterface.h"
#include "queryinterface.h"

#include <QtDBus/QDBusInterface>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusConnectionInterface>
#include <QtDBus/QDBusReply>

#include <QtCore/QEventLoop>
#include <QtCore/QTimer>

#include <KDebug>
#include <KGlobal>


namespace {
    /**
     * Each thread needs its own QDBusConnection. We do it the very easy
     * way and just create a new connection for each client
     * Why does Qt not handle this automatically?
     */
    class QDBusConnectionPerThreadHelper
    {
    public:
        QDBusConnectionPerThreadHelper()
            : m_counter( 0 ) {
        }

        QDBusConnection newConnection() {
            QMutexLocker lock( &m_mutex );
            return QDBusConnection::connectToBus( QDBusConnection::SessionBus,
                                                  QString("NepomukQueryServiceConnection%1").arg(++m_counter) );
        }

    private:
        int m_counter;
        QMutex m_mutex;
    };

    K_GLOBAL_STATIC( QDBusConnectionPerThreadHelper, s_globalDBusConnectionPerThreadHelper )
}


class Nepomuk::Search::QueryServiceClient::Private
{
public:
    Private()
        : queryServiceInterface( 0 ),
          queryInterface( 0 ),
          dbusConnection( s_globalDBusConnectionPerThreadHelper->newConnection() ),
          loop( 0 ) {
    }

    void _k_entriesRemoved( const QStringList& );
    void _k_finishedListing();
    bool handleQueryReply( QDBusReply<QDBusObjectPath> reply );

    org::kde::nepomuk::QueryService* queryServiceInterface;
    org::kde::nepomuk::Query* queryInterface;

    QueryServiceClient* q;

    QDBusConnection dbusConnection;

    QEventLoop* loop;
};


void Nepomuk::Search::QueryServiceClient::Private::_k_entriesRemoved( const QStringList& uris )
{
    QList<QUrl> ul;
    foreach( const QString& s, uris ) {
        ul.append( QUrl( s ) );
    }
    emit q->entriesRemoved( ul );
}


void Nepomuk::Search::QueryServiceClient::Private::_k_finishedListing()
{
    emit q->finishedListing();
    if( loop ) {
        q->close();
    }
}


bool Nepomuk::Search::QueryServiceClient::Private::handleQueryReply( QDBusReply<QDBusObjectPath> r )
{
    if ( r.isValid() ) {
        queryInterface = new org::kde::nepomuk::Query( queryServiceInterface->service(),
                                                       r.value().path(),
                                                       dbusConnection  );
        connect( queryInterface, SIGNAL( newEntries( QList<Nepomuk::Search::Result> ) ),
                 q, SIGNAL( newEntries( QList<Nepomuk::Search::Result> ) ) );
        connect( queryInterface, SIGNAL( entriesRemoved( QStringList ) ),
                 q, SLOT( _k_entriesRemoved( QStringList ) ) );
        connect( queryInterface, SIGNAL( finishedListing() ),
                 q, SLOT( _k_finishedListing() ) );
        // run the listing async in case the event loop below is the only one we have
        // and we need it to handle the signals and list returns results immediately
        QTimer::singleShot( 0, queryInterface, SLOT(list()) );
        return true;
    }
    else {
        kDebug() << "Query failed:" << r.error().message();
        return false;
    }
}


Nepomuk::Search::QueryServiceClient::QueryServiceClient( QObject* parent )
    : QObject( parent ),
      d( new Private() )
{
    d->q = this;

    Nepomuk::Search::registerDBusTypes();

    // we use our own connection to be thread-safe
    d->queryServiceInterface = new org::kde::nepomuk::QueryService( "org.kde.nepomuk.services.nepomukqueryservice",
                                                                    "/nepomukqueryservice",
                                                                    d->dbusConnection );
}


Nepomuk::Search::QueryServiceClient::~QueryServiceClient()
{
    close();
    delete d;
}


bool Nepomuk::Search::QueryServiceClient::query( const QString& query )
{
    close();

    if ( d->queryServiceInterface->isValid() ) {
        return d->handleQueryReply( d->queryServiceInterface->query( query, QStringList() ) );
    }
    else {
        kDebug() << "Could not contact query service.";
        return false;
    }
}


bool Nepomuk::Search::QueryServiceClient::query( const Query& query )
{
    close();

    if ( d->queryServiceInterface->isValid() ) {
        return d->handleQueryReply( d->queryServiceInterface->query( query ) );
    }
    else {
        kDebug() << "Could not contact query service.";
        return false;
    }
}


bool Nepomuk::Search::QueryServiceClient::blockingQuery( const QString& q )
{
    if( query( q ) ) {
        QEventLoop loop;
        d->loop = &loop;
        loop.exec();
        d->loop = 0;
        return true;
    }
    else {
        return false;
    }
}


bool Nepomuk::Search::QueryServiceClient::blockingQuery( const Query& q )
{
    if( query( q ) ) {
        QEventLoop loop;
        d->loop = &loop;
        loop.exec();
        d->loop = 0;
        return true;
    }
    else {
        return false;
    }
}


void Nepomuk::Search::QueryServiceClient::close()
{
    if ( d->queryInterface ) {
        kDebug();
        d->queryInterface->close();
        delete d->queryInterface;
        d->queryInterface = 0;
        if( d->loop )
            d->loop->exit();
    }
}


bool Nepomuk::Search::QueryServiceClient::serviceAvailable()
{
    return QDBusConnection::sessionBus().interface()->isServiceRegistered( "org.kde.nepomuk.services.nepomukqueryservice" );
}

#include "queryserviceclient.moc"
