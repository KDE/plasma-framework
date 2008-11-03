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

#ifndef _NEPOMUK_QUERY_SERVICE_CLIENT_H_
#define _NEPOMUK_QUERY_SERVICE_CLIENT_H_

#include <QtCore/QObject>

#include "nepomukqueryclient_export.h"

class QUrl;

namespace Nepomuk {
    namespace Search {

        class Result;
        class Query;

        /**
         * \class QueryServiceClient queryserviceclient.h Nepomuk/Search/QueryServiceClient
         *
         * \brief Convenience frontend to the %Nepomuk Query DBus Service
         *
         * The QueryServiceClient provides an easy way to access the %Nepomuk Query Service
         * without having to deal with any communication details. By default it monitors 
         * queries for changes.
         *
         * Usage is simple: Create an instance of the client for each search you want to
         * track. Once instance may also be reused for subsequent queries if further updates
         * of the persistent query are not necessary.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */
        class NEPOMUKQUERYCLIENT_EXPORT QueryServiceClient : public QObject
        {
            Q_OBJECT

        public:
            /**
             * Create a new QueryServiceClient instance.
             */
            QueryServiceClient( QObject* parent = 0 );

            /**
             * Desctructor. Closes the query.
             */
            ~QueryServiceClient();

            /**
             * Check if the service is running.
             * \return \p true if the Nepomuk query service is running and could
             * be contacted via DBus, \p false otherwise
             */
            static bool serviceAvailable();

        public Q_SLOTS:
            /**
             * Start a query using the Nepomuk user query language.
             *
             * Results will be reported via newEntries. All results
             * have been reported once finishedListing has been emitted.
             *
             * \return \p true if the query service was found and the query
             * was started. \p false otherwise.
             *
             * \sa QueryParser
             */
            bool query( const QString& query );

            /**
             * Start a query.
             *
             * Results will be reported via newEntries. All results
             * have been reported once finishedListing has been emitted.
             *
             * \return \p true if the query service was found and the query
             * was started. \p false otherwise.
             */
            bool query( const Query& query );

            /**
             * Start a query using the Nepomuk user query language.
             *
             * Results will be reported as with query(const QString&)
             * but a local event loop will be started to block the method
             * call until all results have been listed.
             *
             * The client will be closed after the initial listing. Thus,
             * changes to results will not be reported as it is the case
             * with the non-blocking methods.
             *
             * \return \p true if the query service was found and the query
             * was started. \p false otherwise.
             * 
             * \sa query(const QString&), close()
             */
            bool blockingQuery( const QString& query );

            /**
             * \overload
             *
             * \sa query(const Query&)
             */
            bool blockingQuery( const Query& query );

            /**
             * Close the client, thus stop to monitor the query
             * for changes. Without closing the client it will continue
             * signalling changes to the results.
             *
             * This will also make any blockingQuery return immediately.
             */
            void close();

        Q_SIGNALS:
            /**
             * Emitted for new search results. This signal is emitted both
             * for the initial listing and for changes to the search.
             */
            void newEntries( const QList<Nepomuk::Search::Result>& entries );

            /**
             * Emitted if the search results changed when monitoring a query.
             * \param entries A list of resource URIs identifying the resources
             * that dropped out of the query results.
             */
            void entriesRemoved( const QList<QUrl>& entries );

            /**
             * Emitted when the initial listing has been finished, ie. if all 
             * results have been reported via newEntries. If no further updates
             * are necessary the client should be closed now.
             */
            void finishedListing();

        private:
            class Private;
            Private* const d;

            Q_PRIVATE_SLOT( d, void _k_entriesRemoved( const QStringList& ) )
            Q_PRIVATE_SLOT( d, void _k_finishedListing() )
        };
    }
}

#endif
