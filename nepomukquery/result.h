/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2008 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_SEARCH_RESULT_H_
#define _NEPOMUK_SEARCH_RESULT_H_

#include <QtCore/QSharedDataPointer>
#include <QtCore/QUrl>
#include <QtCore/QList>
#include <QtCore/QHash>

#include <Soprano/Statement>

#include "nepomukquery_export.h"

namespace Nepomuk {
    namespace Search {
        /**
         * \brief A single search result.
         *
         * A search returns a set of Result object.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */
        class NEPOMUKQUERY_EXPORT Result
        {
        public:
            Result();
            Result( const QUrl& uri, double score = 0.0 );
            Result( const Result& );
            ~Result();

            Result& operator=( const Result& );

            double score() const;
            QUrl resourceUri() const;

            void setScore( double score );

            void addRequestProperty( const QUrl& property, const Soprano::Node& value );

            QHash<QUrl, Soprano::Node> requestProperties() const;

            Soprano::Node operator[]( const QUrl& property ) const;
            Soprano::Node requestProperty( const QUrl& property ) const;

            bool operator==( const Result& ) const;

        private:
            class Private;
            QSharedDataPointer<Private> d;
        };
    }
}

#endif
