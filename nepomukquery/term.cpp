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

#include "term.h"

#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QSharedData>
#include <QtCore/QDebug>


class Nepomuk::Search::Term::Private : public QSharedData
{
public:
    Private( Type t = InvalidTerm, Comparator c = Equal )
        : type( t ),
          comparator( c ) {
    }

    Type type;
    Comparator comparator;
    Soprano::LiteralValue value;
    QUrl resource;
    QString field;
    QUrl property;
    QList<Term> subTerms;
};


Nepomuk::Search::Term::Term()
    : d( new Private() )
{
}


Nepomuk::Search::Term::Term( const Term& other )
{
    d = other.d;
}


Nepomuk::Search::Term::Term( const Soprano::LiteralValue& value )
    : d( new Private( LiteralTerm ) )
{
    d->value = value;
}


Nepomuk::Search::Term::Term( const QUrl& value )
    : d( new Private( ResourceTerm ) )
{
    d->resource = value;
}


Nepomuk::Search::Term::Term( const QString& field, const Soprano::LiteralValue& value, Comparator c )
    : d( new Private( ComparisonTerm, c ) )
{
    d->field = field;
    d->subTerms.append( Term( value ) );
}


Nepomuk::Search::Term::Term( const QUrl& field, const Soprano::LiteralValue& value, Comparator c )
    : d( new Private( ComparisonTerm, c ) )
{
    d->property = field;
    d->subTerms.append( Term( value ) );
}


Nepomuk::Search::Term::Term( const QUrl& field, const QUrl& resource )
    : d( new Private( ComparisonTerm ) )
{
    d->property = field;
    d->subTerms.append( Term( resource ) );
}


Nepomuk::Search::Term::~Term()
{
}


Nepomuk::Search::Term& Nepomuk::Search::Term::operator=( const Term& other )
{
    d = other.d;
    return *this;
}


Nepomuk::Search::Term& Nepomuk::Search::Term::operator=( const Soprano::LiteralValue& literal )
{
    d->value = literal;
    d->type = LiteralTerm;
    d->subTerms.clear();
    d->field = QString();
    return *this;
}


bool Nepomuk::Search::Term::isValid() const
{
    switch ( d->type ) {
    case InvalidTerm:
        return false;

    case LiteralTerm:
        return d->value.isValid() && d->subTerms.isEmpty();

    case ResourceTerm:
        return d->resource.isValid() && d->subTerms.isEmpty();

    case AndTerm:
    case OrTerm:
        return !d->subTerms.isEmpty();

    case ComparisonTerm:
        return ( !d->field.isEmpty() || !d->property.isEmpty() ) && ( d->subTerms.count() == 1 );
    }
    // make gcc happy
    return false;
}


Nepomuk::Search::Term::Type Nepomuk::Search::Term::type() const
{
    return d->type;
}


Soprano::LiteralValue Nepomuk::Search::Term::value() const
{
    return d->value;
}


QUrl Nepomuk::Search::Term::resource() const
{
    return d->resource;
}


Nepomuk::Search::Term::Comparator Nepomuk::Search::Term::comparator() const
{
    return d->comparator;
}


QString Nepomuk::Search::Term::field() const
{
    return d->field;
}


QUrl Nepomuk::Search::Term::property() const
{
    return d->property;
}


QList<Nepomuk::Search::Term> Nepomuk::Search::Term::subTerms() const
{
    return d->subTerms;
}


void Nepomuk::Search::Term::setType( Type type )
{
    d->type = type;
}


void Nepomuk::Search::Term::setValue( const Soprano::LiteralValue& v )
{
    d->value = v;
    d->resource = QUrl();
}


void Nepomuk::Search::Term::setResource( const QUrl& res )
{
    d->resource = res;
    d->value = Soprano::LiteralValue();
}


void Nepomuk::Search::Term::setComparator( Comparator c )
{
    d->comparator = c;
}


void Nepomuk::Search::Term::setField( const QString& f )
{
    d->field = f;
    d->property = QUrl();
}


void Nepomuk::Search::Term::setSubTerms( const QList<Term>& terms )
{
    d->subTerms = terms;
}


void Nepomuk::Search::Term::setProperty( const QUrl& p )
{
    d->property = p;
    d->field = QString();
}


void Nepomuk::Search::Term::addSubTerm( const Term& term )
{
    d->subTerms.append( term );
}


namespace {
    bool compareLists( const QList<Nepomuk::Search::Term>& t1, const QList<Nepomuk::Search::Term>& t2 ) {
        // brute-force
        foreach( const Nepomuk::Search::Term& t, t1 ) {
            if ( !t2.contains( t ) ) {
                return false;
            }
        }
        foreach( const Nepomuk::Search::Term& t, t2 ) {
            if ( !t1.contains( t ) ) {
                return false;
            }
        }
        return true;
    }
}

bool Nepomuk::Search::Term::operator==( const Term& other ) const
{
    if ( d->type == other.d->type ) {
        if ( d->type == ComparisonTerm ) {
            return ( d->comparator == other.d->comparator &&
                     compareLists( d->subTerms, other.d->subTerms ) );
        }
        else {
            return d->value == other.d->value &&
                d->resource == other.d->resource &&
                d->field == other.d->field &&
                d->property == other.d->property &&
                compareLists( d->subTerms, other.d->subTerms );
        }
    }

    return false;
}


QDebug operator<<( QDebug dbg, const Nepomuk::Search::Term& term )
{
    if ( term.isValid() ) {
        dbg << "(Term";
        switch( term.type() ) {
        case Nepomuk::Search::Term::LiteralTerm:
            dbg << "literal" << term.value();
            break;
        case Nepomuk::Search::Term::ResourceTerm:
            dbg << "resource" << term.resource();
            break;
        case Nepomuk::Search::Term::AndTerm:
            dbg << "and";
            break;
        case Nepomuk::Search::Term::OrTerm:
            dbg << "or";
            break;
        case Nepomuk::Search::Term::ComparisonTerm:
            dbg << "compare";
            switch( term.comparator() ) {
            case Nepomuk::Search::Term::Contains:
                dbg << ":";
                break;
            case Nepomuk::Search::Term::Equal:
                dbg << "=";
                break;
            case Nepomuk::Search::Term::Greater:
                dbg << ">";
                break;
            case Nepomuk::Search::Term::Smaller:
                dbg << "<";
                break;
            case Nepomuk::Search::Term::GreaterOrEqual:
                dbg << ">=";
                break;
            case Nepomuk::Search::Term::SmallerOrEqual:
                dbg << "<=";
                break;
            }
        default:
            break;
        }
        if ( term.type() == Nepomuk::Search::Term::ComparisonTerm ) {
            if ( term.property().isValid() ) {
                dbg << "Property" << term.property();
            }
            else {
                dbg << "Field:" << term.field();
            }
            dbg << term.subTerms().first();
        }
        if ( term.type() == Nepomuk::Search::Term::AndTerm ||
             term.type() == Nepomuk::Search::Term::OrTerm ) {
            dbg << "Subterms: [";
            foreach( const Nepomuk::Search::Term &t, term.subTerms() ) {
                dbg << t;
            }
            dbg << "]";
        }
        dbg << ")";
    }

    return dbg;
}


uint Nepomuk::Search::qHash( const Nepomuk::Search::Term& term )
{
    switch( term.type() ) {
    case Nepomuk::Search::Term::LiteralTerm:
        return qHash( term.value().toString() );

    case Nepomuk::Search::Term::ComparisonTerm:
        return( qHash( term.property().isValid() ?  term.property().toString() : term.field() )<<16 |
                qHash( term.subTerms().first() )<<8 |
                ( uint )term.comparator() );

    case Nepomuk::Search::Term::AndTerm:
    case Nepomuk::Search::Term::OrTerm: {
        uint h = ( uint )term.type();
        QList<Nepomuk::Search::Term> subTerms = term.subTerms();
        for ( int i = 0; i < subTerms.count(); ++i ) {
            h |= ( qHash( subTerms[i] )<<i );
        }
        return h;
    }

    default:
        return 0;
    }
}
