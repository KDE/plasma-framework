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

#include "dbusoperators.h"

#include <QtDBus/QDBusMetaType>


Q_DECLARE_METATYPE(Nepomuk::Search::Result)
Q_DECLARE_METATYPE(Nepomuk::Search::Term)
Q_DECLARE_METATYPE(Nepomuk::Search::Query)
Q_DECLARE_METATYPE(Soprano::Node)
Q_DECLARE_METATYPE(QList<int>)
Q_DECLARE_METATYPE(QList<Nepomuk::Search::Result>)


void Nepomuk::Search::registerDBusTypes()
{
    qDBusRegisterMetaType<Nepomuk::Search::Result>();
    qDBusRegisterMetaType<QList<Nepomuk::Search::Result> >();
    qDBusRegisterMetaType<Nepomuk::Search::Term>();
    qDBusRegisterMetaType<Nepomuk::Search::Query>();
    qDBusRegisterMetaType<Soprano::Node>();
}


QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::Search::Result& result )
{
    //
    // Signature: (sda{s(isss)})
    //

    arg.beginStructure();

    arg << result.resourceUri().toString() << result.score();

    arg.beginMap( QVariant::String, qMetaTypeId<Soprano::Node>() );

    QHash<QUrl, Soprano::Node> rp = result.requestProperties();
    for ( QHash<QUrl, Soprano::Node>::const_iterator it = rp.constBegin(); it != rp.constEnd(); ++it ) {
        arg.beginMapEntry();
        arg << it.key().toString() << it.value();
        arg.endMapEntry();
    }

    arg.endMap();

    arg.endStructure();

    return arg;
}


const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::Search::Result& result )
{
    //
    // Signature: (sda{s(isss)})
    //

    arg.beginStructure();
    QString uri;
    double score = 0.0;

    arg >> uri >> score;
    result = Nepomuk::Search::Result( QUrl( uri ), score );

    arg.beginMap();
    while ( !arg.atEnd() ) {
        QString rs;
        Soprano::Node node;
        arg.beginMapEntry();
        arg >> rs >> node;
        arg.endMapEntry();
        result.addRequestProperty( QUrl( rs ), node );
    }
    arg.endMap();

    arg.endStructure();

    return arg;
}


QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::Search::Term& term )
{
    //
    // Signature: (ii(isss)sss)
    // i      -> type
    // i      -> comparator type
    // (isss) -> Soprano::LiteralValue encoded as a Soprano::Node for simplicity
    // s      -> resource
    // s      -> field
    // s      -> property
    //

    arg.beginStructure();
    arg << ( int )term.type()
        << ( int )term.comparator()
        << Soprano::Node( term.value() )
        << term.resource().toString()
        << term.field()
        << term.property().toString();
    arg.endStructure();

    return arg;
}


const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::Search::Term& term )
{
    //
    // Signature: (ii(isss)sss)
    // i      -> type
    // i      -> comparator type
    // (isss) -> Soprano::LiteralValue encoded as a Soprano::Node for simplicity
    // s      -> resource
    // s      -> field
    // s      -> property
    //

    arg.beginStructure();
    int type = Nepomuk::Search::Term::InvalidTerm;
    int comparator = Nepomuk::Search::Term::Equal;
    Soprano::Node valueNode;
    QString resource, field, property;
    arg >> type
        >> comparator
        >> valueNode
        >> resource
        >> field
        >> property;
    term.setType( Nepomuk::Search::Term::Type( type ) );
    term.setComparator( Nepomuk::Search::Term::Comparator( comparator ) );
    if ( valueNode.isLiteral() )
        term.setValue( valueNode.literal() );
    if ( !resource.isEmpty() )
        term.setResource( QUrl( resource ) );
    if ( !field.isEmpty() )
        term.setField( field );
    if ( !property.isEmpty() )
        term.setProperty( QUrl( property ) );
    arg.endStructure();

    return arg;
}


// streaming a Query object is a bit tricky as it is a set of nested Term objects
// DBus does not allow arbitrary nesting of objects, thus we use a little trick:
// We store all used Term objects in a list and use integer indices pointing into
// this list to describe the nesting within the Term objects. This also means that
// a Term's subTerm list is replaced with a list of indices
namespace {
    /**
     * Build term relations for the last term in the list
     */
    void buildTermRelations( QList<Nepomuk::Search::Term>& terms, QHash<int, QList<int> >& termRelations ) {
        QList<Nepomuk::Search::Term> subTerms = terms.last().subTerms();
        int termIndex = terms.count()-1;
        for ( int i = 0; i < subTerms.count(); ++i ) {
            terms.append( subTerms[i] );
            termRelations[termIndex].append( terms.count()-1 );
            buildTermRelations( terms, termRelations );
        }
    }
}

QDBusArgument& operator<<( QDBusArgument& arg, const Nepomuk::Search::Query& query )
{
    //
    // Signature: (isa(ii(isss)sss)a{iai}ia{sb})
    // i      -> type
    // s      -> sparql query
    // a(ii(isss)sss)    -> array of terms (first is root term)
    // a{iai} -> hash of term relations
    // i      -> limit
    // a{sb}  -> request properties
    //

    arg.beginStructure();

    arg << ( int )query.type() << query.sparqlQuery();

    QList<Nepomuk::Search::Term> terms;
    QHash<int, QList<int> > termRelations;
    if ( query.type() == Nepomuk::Search::Query::PlainQuery ) {
        terms.append( query.term() );
        buildTermRelations( terms, termRelations );
    }
    arg << terms;

    arg.beginMap( QVariant::Int, qMetaTypeId<QList<int> >() );
    for( QHash<int, QList<int> >::const_iterator it = termRelations.constBegin();
         it != termRelations.constEnd(); ++it ) {
        arg.beginMapEntry();
        arg << it.key() << it.value();
        arg.endMapEntry();
    }
    arg.endMap();
    arg << query.limit();

    arg.beginMap( QVariant::String, QVariant::Bool );
    QList<Nepomuk::Search::Query::RequestProperty> requestProperties = query.requestProperties();
    foreach( const Nepomuk::Search::Query::RequestProperty& rp, requestProperties ) {
        arg.beginMapEntry();
        arg << rp.first.toString() << rp.second;
        arg.endMapEntry();
    }
    arg.endMap();

    arg.endStructure();

    return arg;
}


namespace {
    Nepomuk::Search::Term rebuildTermFromTermList( const QList<Nepomuk::Search::Term>& terms,
                                                   const QHash<int, QList<int> >& termRelations,
                                                   int index = 0 ) {
        Nepomuk::Search::Term root = terms[index];
        foreach( int i, termRelations[index] ) {
            root.addSubTerm( rebuildTermFromTermList( terms, termRelations, i ) );
        }
        return root;
    }
}

const QDBusArgument& operator>>( const QDBusArgument& arg, Nepomuk::Search::Query& query )
{
    //
    // Signature: (isa(ii(isss)sss)a{iai}ia{sb})
    // i      -> type
    // s      -> sparql query
    // a(ii(isss)sss)    -> array of terms (first is root term)
    // a{iai} -> hash of term relations
    // i      -> limit
    // a{sb}  -> request properties
    //

    arg.beginStructure();

    int type = Nepomuk::Search::Query::InvalidQuery;
    QString sparqlQuery;
    QList<Nepomuk::Search::Term> terms;
    QHash<int, QList<int> > termRelations;
    int limit = 0;

    arg >> type
        >> sparqlQuery
        >> terms;

    arg.beginMap();
    while ( !arg.atEnd() ) {
        int termIndex = 0;
        QList<int> indices;
        arg.beginMapEntry();
        arg >> termIndex >> indices;
        arg.endMapEntry();
        termRelations.insert( termIndex, indices );
    }
    arg.endMap();

    arg >> limit;

    arg.beginMap();
    while ( !arg.atEnd() ) {
        QString prop;
        bool optional = true;
        arg.beginMapEntry();
        arg >> prop >> optional;
        arg.endMapEntry();
        query.addRequestProperty( QUrl( prop ), optional );
    }
    arg.endMap();

    arg.endStructure();

    if ( Nepomuk::Search::Query::Type( type ) == Nepomuk::Search::Query::PlainQuery ) {
        query.setTerm( rebuildTermFromTermList( terms, termRelations ) );
    }
    else {
        query.setSparqlQuery( sparqlQuery );
    }
    query.setLimit( limit );

    return arg;
}


QDBusArgument& operator<<( QDBusArgument& arg, const Soprano::Node& node )
{
    //
    // Signature: (isss)
    //
    arg.beginStructure();
    arg << ( int )node.type() << node.toString() << node.language() << node.dataType().toString();
    arg.endStructure();
    return arg;
}


const QDBusArgument& operator>>( const QDBusArgument& arg, Soprano::Node& node )
{
    //
    // Signature: (isss)
    //
    arg.beginStructure();
    int type;
    QString value, language, dataTypeUri;
    arg >> type >> value >> language >> dataTypeUri;
    if ( type == Soprano::Node::LiteralNode ) {
        node = Soprano::Node( Soprano::LiteralValue::fromString( value, dataTypeUri ), language );
    }
    else if ( type == Soprano::Node::ResourceNode ) {
        node = Soprano::Node( QUrl( value ) );
    }
    else if ( type == Soprano::Node::BlankNode ) {
        node = Soprano::Node( value );
    }
    else {
        node = Soprano::Node();
    }
    arg.endStructure();
    return arg;
}
