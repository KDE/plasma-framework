/*  This file is part of the KDE project
    Copyright (C) 2006-2007 Will Stephenson <wstephenson@kde.org>
    Copyright (C) 2006-2007 Kevin Ottens <ervin@kde.org>

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

#include <QtNetwork/QAbstractSocket>
#include <QtCore/QTimer>

#include <kglobal.h>

#include "networking_p.h"
#include "networking.h"
#include "org_kde_solid_networking.h"

K_GLOBAL_STATIC(Solid::Control::NetworkingPrivate, globalNetworkControl)

Solid::Control::NetworkingPrivate::NetworkingPrivate() : iface( 
        new OrgKdeSolidNetworkingInterface( "org.kde.Solid.Networking",
            "/status",
            QDBusConnection::sessionBus(),
            this ) )
{
}

Solid::Control::NetworkingPrivate::~NetworkingPrivate()
{
}uint Solid::Control::NetworkingPrivate::requestConnection( QObject * receiver, const char * member )
{
    connect( this, SIGNAL( connectionResult( bool ) ), receiver, member );
    return iface->requestConnection();
}

void Solid::Control::NetworkingPrivate::releaseConnection()
{
    iface->releaseConnection();
}

Solid::Control::Networking::Result Solid::Control::NetworkingPrivate::beginManagingSocket( QAbstractSocket * socket, int autoDisconnectTimeout )
{
    mManagedSockets.insert( socket, new ManagedSocketContainer( socket, autoDisconnectTimeout ) );
    return Solid::Control::Networking::Accepted;
}

void Solid::Control::NetworkingPrivate::stopManagingSocket( QAbstractSocket * socket )
{
    ManagedSocketContainer * removed = mManagedSockets.take( socket );
    delete removed;
}

Solid::Control::Networking::Result Solid::Control::Networking::requestConnection( QObject * receiver, const char * member )
{
    return static_cast<Solid::Control::Networking::Result>( globalNetworkControl->requestConnection( receiver, member ) );
}

void Solid::Control::Networking::releaseConnection()
{
    globalNetworkControl->releaseConnection();
}

/*=========================================================================*/


Solid::Control::ManagedSocketContainer::ManagedSocketContainer( QAbstractSocket * socket, int autoDisconnectTimeout ) : mSocket( socket ), mAutoDisconnectTimer( 0 )
{
    if ( autoDisconnectTimeout >= 0 )
    {
        mAutoDisconnectTimer = new QTimer( this );
        mAutoDisconnectTimer->setSingleShot( true );
        mAutoDisconnectTimer->setInterval( autoDisconnectTimeout );
        connect( mAutoDisconnectTimer, SIGNAL( timeout() ), SLOT( autoDisconnect() ) );
    }
    // react to network management events
    connect( Solid::Networking::notifier(), SIGNAL( statusChanged( uint ) ), this, SLOT( networkStatusChanged( Networking::Status ) ) );

    if ( socket )
    {
        // react to socket events
        connect( socket, SIGNAL( destroyed() ), SLOT( socketDestroyed() ) );
        connect( socket, SIGNAL( error( QAbstractSocket::SocketError ) ), SLOT( socketError( QAbstractSocket::SocketError ) ) );
        connect( socket, SIGNAL( stateChanged( QAbstractSocket::SocketState ) ), SLOT( socketStateChanged( QAbstractSocket::SocketState ) ) );
        // initialise our state from that of the socket
        switch ( socket->state() )
        {
            case QAbstractSocket::UnconnectedState:
                mState = SocketUnconnected;
                break;
            case QAbstractSocket::HostLookupState:
            case QAbstractSocket::ConnectingState:
                mState = SocketConnecting;
                break;
            case QAbstractSocket::ConnectedState:
            case QAbstractSocket::ClosingState:
                mState = SocketConnected;
                break;
            default:
                mState = SocketUnconnected;
        }
    }
}

void Solid::Control::ManagedSocketContainer::networkStatusChanged( Solid::Networking::Status netStatus )
{
    switch ( mState )
    {
        case SocketUnconnected:
            break;
        case SocketConnecting:
            break;
        case AwaitingNetworkConnection:
            switch ( netStatus )
            {
                case Solid::Networking::Connected:
                    performConnectToHost();
                    break;
                default:
                    //do nothing
                    ;
            }
            break;
        case SocketConnected:
            switch ( netStatus )
            {
                case Solid::Networking::Unconnected:
                case Solid::Networking::Disconnecting:
                    mState = DisconnectWait;
                    if ( mAutoDisconnectTimer )
                    {
                        mAutoDisconnectTimer->start();
                    }
                    break;
                default:
                    // do nothing
                    ;
            }
            break;
        case DisconnectWait:
            switch ( netStatus )
            {
                case Solid::Networking::Connected:
                    // RECOVERED
                    mState = SocketConnected;
                    if ( mAutoDisconnectTimer )
                    {
                        mAutoDisconnectTimer->stop();
                    }
                    break;
                default:
                    // do nothing
                    ;
            }
            break;
    }
}

void Solid::Control::ManagedSocketContainer::socketError( QAbstractSocket::SocketError socketError )
{
    switch ( mState )
    {
        case SocketUnconnected:
            break;
        case SocketConnecting:
            switch ( socketError )
            {
                case QAbstractSocket::HostNotFoundError:
                case QAbstractSocket::NetworkError:
                    // socket tried to resolve and failed
                    // Either the host doesn't exist at all
                    // or the resolve failed because we're offline, so request that we go online
                    if ( Solid::Networking::status() != Solid::Networking::Connected )
                    {
                        mState = AwaitingNetworkConnection;
                        globalNetworkControl->requestConnection();
                    }
                    else
                    {
                        mState = SocketUnconnected;
                    }
                    break;
                default:
                    mState = SocketUnconnected;
            }
            break;
        case AwaitingNetworkConnection:
        case SocketConnected:
            // setup automatic reconnect now when/if we impl this
        case DisconnectWait:
            // maybe check the socket state that it thinks it is now unconnected too
            mState = SocketUnconnected;
            break;
    }
}

void Solid::Control::ManagedSocketContainer::socketStateChanged( QAbstractSocket::SocketState socketState )
{
    switch ( mState )
    {
        case SocketUnconnected:
            switch ( socketState )
            {
                case QAbstractSocket::HostLookupState:
                case QAbstractSocket::ConnectingState:
                    // the socket is trying to connect, cache its connection parameter in case it
                    // fails and we want to reconnect it when the network is available.
                    mState = SocketConnecting;
                    if ( mSocket )
                    {
                        mPeerName = mSocket->peerName();
                        mPeerPort = mSocket->peerPort();
                        mSocketOpenMode = mSocket->openMode();
                    }
                    break;
                default:
                    ;
            }
            break;
        case SocketConnecting:
            switch ( socketState )
            {
                case QAbstractSocket::HostLookupState:
                case QAbstractSocket::ConnectingState:
                    // still connecting, do nothing
                    break;
                case QAbstractSocket::BoundState:
                case QAbstractSocket::ConnectedState:
                case QAbstractSocket::ListeningState:
                    // socket connected unaided
                    mState = SocketConnected;
                    break;
                case QAbstractSocket::UnconnectedState:
                    // this state is preceded by ClosingState, so no action needed
                    break;
                case QAbstractSocket::ClosingState:
                    // it's unlikely that an unconnected socket can go to this state, but...
                    mState = SocketUnconnected;
                    break;
            }
            break;
        case AwaitingNetworkConnection:
            switch ( socketState )
            {
                case QAbstractSocket::ConnectedState:
                    // somehow the socket connected itself when it shouldn't have been able to.
                    mState = SocketConnected;

                    break;
                default:
                    //do nothing
                    ;
            }
            break;
        case SocketConnected:
            switch ( socketState )
            {
                case QAbstractSocket::UnconnectedState:
                case QAbstractSocket::ClosingState:
                    // socket disconnected
                    mState = SocketUnconnected;
                    break;
                case QAbstractSocket::ConnectingState:
                    mState = SocketConnected;
                    break;
                default:
                    ;
            }
            break;
        case DisconnectWait:
            switch ( socketState )
            {
                case QAbstractSocket::UnconnectedState:
                case QAbstractSocket::ClosingState:
                    // socket disconnected anyway
                    mState = SocketUnconnected;
                    if ( mAutoDisconnectTimer )
                    {
                        mAutoDisconnectTimer->stop();
                    }
                    break;
                default:
                    break;
            }
            break;
    }
}

void Solid::Control::ManagedSocketContainer::autoDisconnect()
{
    if ( mAutoDisconnectTimer && mSocket )
        mSocket->disconnectFromHost();
}

void Solid::Control::ManagedSocketContainer::socketDestroyed()
{
    mSocket = 0;
    delete mAutoDisconnectTimer;
    mAutoDisconnectTimer = 0;
    disconnect( globalNetworkControl );
}

void Solid::Control::ManagedSocketContainer::performConnectToHost()
{
    if ( mSocket )
    {
        mSocket->connectToHost( mPeerName, mPeerPort, mSocketOpenMode );
    }
}

#include "networking_p.moc"
#include "networking.moc"
