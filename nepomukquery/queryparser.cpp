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

#include "queryparser.h"
#include "query.h"
#include "term.h"

#include <QtCore/QRegExp>
#include <QtCore/QSet>

#include <KDebug>
#include <KLocale>


/* Advanced queries:
 * select distinct ?r ?p ?x ?label ?comment where { { ?r ?p ?x . } UNION { ?r ?p ?r2 . ?r2 ?p2 ?x . } . FILTER(isLiteral(?x)) . FILTER REGEX(STR(?p),'hastag','i') . FILTER REGEX(STR(?x),'nepomuk','i') . OPTIONAL { { ?r <http://www.w3.org/2000/01/rdf-schema#label> ?label } UNION { ?r <http://www.semanticdesktop.org/ontologies/2007/08/15/nao#prefLabel> ?label . } UNION { ?r <http://freedesktop.org/standards/xesam/1.0/core#name> ?label . } . ?r <http://www.w3.org/2000/01/rdf-schema#comment> ?comment . } . }
 */

namespace {
    // a field differs from a plain term in that it does never allow comparators
    QString s_fieldNamePattern( "([^\\s\"':=<>]+|(?:([\"'])[^\"':=<>]+\\%1))" );
    QString s_plainTermPattern( "([^\\s\"':=<>]+|(?:([\"'])[^\"']+\\%1))" );
    QString s_inExclusionPattern( "([\\+\\-]?)" );
    QString s_uriPattern( "<([^<>]+)>" );
    QString s_comparatorPattern( "(:|\\<=|\\>=|=|\\<|\\>)" );

    // match a simple search text
    // captures: 1 - The optional + or - sign (may be empty)
    //           2 - the search text (including optional paranthesis)
    QRegExp s_plainTermRx( s_inExclusionPattern + s_plainTermPattern.arg( 3 ) );

    // match a field search term: fieldname + relation (:, =, etc) + search text with optional paranthesis
    // captures: 1 - The optional + or - sign (may be empty)
    //           2 - fieldname
    //           3 - relation
    //           4 - search text (including optional paranthesis)
    QRegExp s_fieldRx( s_inExclusionPattern + s_fieldNamePattern.arg( 3 ) + s_comparatorPattern + s_plainTermPattern.arg( 6 ) );

    // match a property URI search term: property URI + relation (:, =, etc) + search text with optional paranthesis
    // captures: 1 - The optional + or - sign (may be empty)
    //           2 - property URI
    //           3 - relation
    //           4 - search text (including optional paranthesis)
    QRegExp s_propertyRx( s_inExclusionPattern + s_uriPattern + s_comparatorPattern + s_plainTermPattern.arg( 5 ) );

    // match a property URI search term: property URI + relation (:, =, etc) + resource URI
    // captures: 1 - The optional + or - sign (may be empty)
    //           2 - property URI
    //           3 - resource URI
    QRegExp s_resourceRx( s_inExclusionPattern + s_uriPattern + "(?::|=)" + s_uriPattern );

    QRegExp s_fieldFieldRx( s_inExclusionPattern + s_fieldNamePattern.arg( 3 ) + s_comparatorPattern + "\\(" +  s_fieldNamePattern.arg( 6 ) + s_comparatorPattern + s_plainTermPattern.arg( 9 ) + "\\)" );

    Nepomuk::Search::Term::Comparator fieldTypeRelationFromString( const QString& s ) {
        if ( s == "=" ) {
            return Nepomuk::Search::Term::Equal;
        }
        else if ( s == ":" ) {
            return Nepomuk::Search::Term::Contains;
        }
        else if ( s == ">" ) {
            return Nepomuk::Search::Term::Greater;
        }
        else if ( s == "<" ) {
            return Nepomuk::Search::Term::Smaller;
        }
        else if ( s == ">=" ) {
            return Nepomuk::Search::Term::GreaterOrEqual;
        }
        else if ( s == "<=" ) {
            return Nepomuk::Search::Term::SmallerOrEqual;
        }
        else {
            kDebug() << "FIXME: Unsupported relation:" << s;
            return Nepomuk::Search::Term::Equal;
        }
    }

    QString stripQuotes( const QString& s ) {
        if ( s[0] == '\'' ||
             s[0] == '\"' ) {
            return s.mid( 1 ).left( s.length()-2 );
        }
        else {
            return s;
        }
    }

    QUrl tryToBeIntelligentAboutParsingUrl( const QString& s ) {
        if ( s.contains( '%' ) && !s.contains( '/' ) ) {
            return QUrl::fromEncoded( s.toAscii() );
        }
        else {
            return QUrl( s );
        }
    }

    Soprano::LiteralValue createLiteral( const QString& s ) {
        bool b = false;
        int i = s.toInt( &b );
        if ( b )
            return Soprano::LiteralValue( i );
        double d = s.toDouble( &b );
        if ( b )
            return Soprano::LiteralValue( d );
        return s;
    }
}


Nepomuk::Search::Query Nepomuk::Search::QueryParser::parseQuery( const QString& query )
{
    QueryParser parser;
    return parser.parse( query );
}


class Nepomuk::Search::QueryParser::Private
{
public:
    QSet<QString> andKeywords;
    QSet<QString> orKeywords;
};


Nepomuk::Search::QueryParser::QueryParser()
    : d( new Private() )
{
    QString andListStr = i18nc( "Boolean AND keyword in desktop search strings. You can add several variants separated by spaces, e.g. retain the English one alongside the translation; keywords are not case sensitive. Make sure there is no conflict with the OR keyword.", "and" );
    foreach ( const QString &andKeyword, andListStr.split( " ", QString::SkipEmptyParts ) ) {
        d->andKeywords.insert( andKeyword.toLower() );
    }
    QString orListStr = i18nc( "Boolean OR keyword in desktop search strings. You can add several variants separated by spaces, e.g. retain the English one alongside the translation; keywords are not case sensitive. Make sure there is no conflict with the AND keyword.", "or" );
    foreach ( const QString &orKeyword, orListStr.split( " ", QString::SkipEmptyParts ) ) {
        d->orKeywords.insert( orKeyword.toLower() );
    }
}


Nepomuk::Search::QueryParser::~QueryParser()
{
    delete d;
}


Nepomuk::Search::Query Nepomuk::Search::QueryParser::parse( const QString& query )
{
    // TODO: a "real" parser which can handle all of the Xesam user language
    //       This one for example does not handle nesting at all.

    QList<Term> terms;

    bool inOrBlock = false;
    bool inAndBlock = false;

    int pos = 0;
    while ( pos < query.length() ) {
        // skip whitespace
        while ( pos < query.length() && query[pos].isSpace() ) {
            kDebug() << "Skipping space at" << pos;
            ++pos;
        }

        Term term;

        if ( pos < query.length() ) {
            if ( s_resourceRx.indexIn( query, pos ) == pos ) {
                // FIXME: honour the +-
                kDebug() << "matched resource term at" << pos << s_resourceRx.cap( 0 );
                term = Term( tryToBeIntelligentAboutParsingUrl( s_resourceRx.cap( 2 ) ),
                             tryToBeIntelligentAboutParsingUrl( s_resourceRx.cap( 3 ) ) );
                pos += s_resourceRx.matchedLength();
            }
            else if ( s_propertyRx.indexIn( query, pos ) == pos ) {
                // FIXME: honour the +-
                kDebug() << "matched property term at" << pos << s_propertyRx.cap( 0 );
                term.setProperty( tryToBeIntelligentAboutParsingUrl( s_propertyRx.cap( 2 ) ) );
                term.addSubTerm( Term( createLiteral( stripQuotes( s_propertyRx.cap( 4 ) ) ) ) );
                QString comparator = s_propertyRx.cap( 3 );
                term.setType( Term::ComparisonTerm );
                term.setComparator( fieldTypeRelationFromString( comparator ) );
                pos += s_propertyRx.matchedLength();
            }
            else if ( s_fieldFieldRx.indexIn( query, pos ) == pos ) {
                kDebug() << "matched field field term at" << pos
                         << s_fieldFieldRx.cap( 0 )
                         << s_fieldFieldRx.cap( 2 )
                         << s_fieldFieldRx.cap( 4 )
                         << s_fieldFieldRx.cap( 5 )
                         << s_fieldFieldRx.cap( 7 )
                         << s_fieldFieldRx.cap( 8 );
                term.setField( stripQuotes( s_fieldFieldRx.cap( 2 ) ) );
                QString comparator = s_fieldFieldRx.cap( 4 );
                term.setType( Term::ComparisonTerm );
                term.setComparator( fieldTypeRelationFromString( comparator ) );
                term.addSubTerm( Term( stripQuotes( s_fieldFieldRx.cap( 5 ) ), s_fieldFieldRx.cap( 8 ), fieldTypeRelationFromString( s_fieldFieldRx.cap( 7 ) ) ) );
                pos += s_fieldFieldRx.matchedLength();
            }
            else if ( s_fieldRx.indexIn( query, pos ) == pos ) {
                // FIXME: honour the +-
                kDebug() << "matched field term at" << pos << s_fieldRx.cap( 0 ) << s_fieldRx.cap( 2 ) << s_fieldRx.cap( 4 ) << s_fieldRx.cap( 5 );
                term.setField( stripQuotes( s_fieldRx.cap( 2 ) ) );
                term.addSubTerm( Term( createLiteral( stripQuotes( s_fieldRx.cap( 5 ) ) ) ) );
                QString comparator = s_fieldRx.cap( 4 );
                term.setType( Term::ComparisonTerm );
                term.setComparator( fieldTypeRelationFromString( comparator ) );
                pos += s_fieldRx.matchedLength();
            }
            else if ( s_plainTermRx.indexIn( query, pos ) == pos ) {
                // FIXME: honour the +-
                QString value = stripQuotes( s_plainTermRx.cap( 2 ) );
                if ( d->orKeywords.contains( value.toLower() ) ) {
                    inOrBlock = true;
                }
                else if ( d->andKeywords.contains( value.toLower() ) ) {
                    inAndBlock = true;
                }
                else {
                    kDebug() << "matched literal at" << pos << value;
                    term = Term( Soprano::LiteralValue( value ) );
                }
                pos += s_plainTermRx.matchedLength();
            }
            else {
                kDebug() << "Invalid query at" << pos << query;
                return Term();
            }

            if ( term.isValid() ) {
                if ( inOrBlock && !terms.isEmpty() ) {
                    Term orTerm;
                    orTerm.setType( Term::OrTerm );
                    orTerm.addSubTerm( terms.takeLast() );
                    orTerm.addSubTerm( term );
                    terms.append( orTerm );
                }
                else if ( inAndBlock && !terms.isEmpty() ) {
                    Term andTerm;
                    andTerm.setType( Term::AndTerm );
                    andTerm.addSubTerm( terms.takeLast() );
                    andTerm.addSubTerm( term );
                    terms.append( andTerm );
                }
                else {
                    terms.append( term );
                }
            }
        }
    }

    if ( terms.count() == 1 ) {
        return terms[0];
    }
    else if ( terms.count() > 0 ) {
        Term t;
        t.setType( Term::AndTerm );
        t.setSubTerms( terms );
        return t;
    }
    else {
        return Term();
    }
}
