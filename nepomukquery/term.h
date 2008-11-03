/*
   This file is part of the Nepomuk KDE project.
   Copyright (C) 2007 Sebastian Trueg <trueg@kde.org>

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

#ifndef _NEPOMUK_SEARCH_TERM_H_
#define _NEPOMUK_SEARCH_TERM_H_

#include <QtCore/QList>
#include <QtCore/QUrl>
#include <QtCore/QSharedDataPointer>

#include <Soprano/LiteralValue>

#include "nepomukquery_export.h"


namespace Nepomuk {
    namespace Search {
        /**
         * \class Term term.h nepomuk/term.h
         *
         * \brief A Query constist of Terms.
         *
         * Queries are build from Term instances. A Term can have one of multiple
         * types and subterms. See Term::Type for details on the different Term types.
         *
         * \author Sebastian Trueg <trueg@kde.org>
         */
        class NEPOMUKQUERY_EXPORT Term
        {
        public:
            /**
             * Each search term has a type.
             */
            enum Type {
                InvalidTerm,    /**< An invalid term does not do anything */
                /**
                 * A literal term is the simplest form of Term. It matches all resource
                 * that contain the value.
                 *
                 * It is also used to specify literal values in comparison Terms such
                 * as ContainsTerm or EqualityTerm.
                 */
                LiteralTerm,

                /**
                 * A resource term matches one resource by URI.
                 *
                 * It is also used to specify a resource in EqualityTerm Terms.
                 */
                ResourceTerm,

                /**
                 * Match all resources that match all sub terms.
                 */
                AndTerm,

                /**
                 * Match all resources that match one of the sub terms.
                 */
                OrTerm,

                /**
                 * A comparison. The comparison operator needs to be specified in addition.
                 * For specifying the %property the same applies as for ContainsTerm.
                 *
                 * A single subterm specifies the resource or value to match (resource terms
                 * can only be matched via Equal or Contains.
                 */
                ComparisonTerm
            };

            enum Comparator {
                Contains,
                Equal,
                Greater,
                Smaller,
                GreaterOrEqual,
                SmallerOrEqual
            };

            /**
             * Constructs an invalid term.
             */
            Term();

            /**
             * Copy constructor.
             */
            Term( const Term& other );

            /**
             * Construct a literal term.
             */
            Term( const Soprano::LiteralValue& value );

            /**
             * Construct a resource term.
             */
            Term( const QUrl& resource );

            /**
             * Construct a Contains ComparisonTerm term.
             * \param field A string that will be matched against a field label
             * \param value A value that will be matched against the field value. Unsupported
             *        types are converted to string.
             * \param comparator The Comparator to use
             */
            Term( const QString& field, const Soprano::LiteralValue& value, Comparator c = Contains );

            /**
             * Construct a Contains ComparisonTerm term.
             * \param field The exact field to match
             * \param value A value that will be matched against the field value. Unsupported
             *        types are converted to string.
             * \param comparator The Comparator to use
             */
            Term( const QUrl& field, const Soprano::LiteralValue& value, Comparator c = Contains );

            /**
             * Construct an EqualityTerm term.
             * \param field The exact field to match
             * \param value The resource that should be matched.
             */
            Term( const QUrl& field, const QUrl& resource );

            /**
             * Destructor
             */
            ~Term();

            /**
             * Assign another term to this one.
             */
            Term& operator=( const Term& other );

            /**
             * Make the term a literal term.
             */
            Term& operator=( const Soprano::LiteralValue& other );

            /**
             * \return \p true if the Term is valid.
             */
            bool isValid() const;

            /**
             * \return the Term type.
             *
             * \sa setType
             */
            Type type() const;

            /**
             * The literal value of a LiteralTerm.
             *
             * \sa setValue
             */
            Soprano::LiteralValue value() const;

            /**
             * The resource of a ResourceTerm
             *
             * \sa setResource
             */
            QUrl resource() const;

            /**
             * The Comparator used by ComparisonTerm Terms.
             *
             * \sa setComparator
             */
            Comparator comparator() const;

            /**
             * A property name used for ComparisonTerm Terms. Will be matched against
             * the rdfs:label to find the corresponding property.
             *
             * \sa setField, property, setProperty
             */
            QString field() const;

            /**
             * A property used for ComparisonTerm Terms.
             *
             * \sa setProperty, field, setField
             */
            QUrl property() const;

            /**
             * The sub terms  used by AndTerm, OrTerm, and ComparisonTerm.
             *
             * \sa setSubTerms, addSubTerm
             */
            QList<Term> subTerms() const;

            /**
             * Set the type of the Term
             */
            void setType( Type );

            /**
             * Set the value of a LiteralTerm
             */
            void setValue( const Soprano::LiteralValue& );

            /**
             * Set the resource of a ResourceTerm
             */
            void setResource( const QUrl& );

            /**
             * Defaults to Equal
             */
            void setComparator( Comparator );

            /**
             * Set the property label in case the exact
             * property is not known. Will be mached against
             * the property's rdfs:label.
             *
             * \sa field, setProperty, property
             */
            void setField( const QString& );
            
            /**
             * Set the property for ComparisonTerm
             * Terms.
             *
             * If the exact property is not known use setField.
             *
             * \sa property
             */
            void setProperty( const QUrl& );

            /**
             * Set the subterms used by AndTerm, OrTerm, and ComparisonTerm.
             *
             * \sa addSubTerm
             */
            void setSubTerms( const QList<Term>& );

            /**
             * Add a subterm used by AndTerm, OrTerm, and ComparisonTerm.
             *
             * \sa setSubTerm
             */
            void addSubTerm( const Term& );

            /**
             * Comparison operator.
             */
            bool operator==( const Term& ) const;

        private:
            class Private;
            QSharedDataPointer<Private> d; 
        };

        NEPOMUKQUERY_EXPORT uint qHash( const Nepomuk::Search::Term& );
    }
}

NEPOMUKQUERY_EXPORT QDebug operator<<( QDebug, const Nepomuk::Search::Term& );

#endif
