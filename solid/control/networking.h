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

#ifndef SOLID_CONTROL_NETWORKING_H
#define SOLID_CONTROL_NETWORKING_H

#include <QtCore/QObject>

#include "solid_control_export.h"
class QAbstractSocket;

namespace Solid
{
namespace Control
{
    /**
     * This class allow to query the underlying system to discover the available
     * network interfaces and reachable network.It has also the
     * responsibility to notify when a network interface or a network appear or disappear.
     *
     * It's the unique entry point for network management. Applications should use
     * it to find network interfaces, or to be notified about network related changes.
     *
     * Note that it's implemented as a singleton and encapsulates the backend logic.
     */
    namespace Networking
    {
        /**
         * Describe the result of a connection request
         * - Accepted : the request was accepted and is being acted upon
         * - AlreadyConnected : the system was already connected
         * - Denied : the request was denied
         */
        enum Result { Accepted, AlreadyConnected, Denied };

        /**
         * Magic network management for application's sockets.
         * When the socket begins to connect it will automatically bring up
         * networking, if not already available.
         * When the network disconnects, optionally disconnects the socket early
         * so that an application may continue.
         * @param socket the socket to manage.
         * @param autoDisconnectTimeout wait this many milliseconds after receiving a disconnected
         * event from the networking subsystem before disconnecting the socket. A value of 0 means
         * never automatically disconnect.
         * @return whether the management request succeeded.
         */
        SOLIDCONTROL_EXPORT Result beginManagingSocket( QAbstractSocket * socket, uint autoDisconnectTimeout = 0 );

        /**
         * Remove the socket from the list of sockets to manage.  The socket's state is unaltered.
         * @param socket the socket to stop managing.
         */
        SOLIDCONTROL_EXPORT void stopManagingSocket( QAbstractSocket * socket );

        /**
         * Requests that the networking subsystem makes a connection.  If an on-demand connection
         * is started as a result of this request, the connection is refcounted and KDE's use of
         * the connection is dropped when the last application uses it calls @ref
         * releaseConnection().
         * Optionally, pass in a QObject and slot to call on it, to receive notification when the
         * connection is available or not.  The slot may take a Solid::Networking::Status to 
         * indicate success or failure.
         *
         * @param receiver the QObject to call a slot on.
         * @param member the slot to call.
         * @return a Result indication whether the request was accepted.
         */
        SOLIDCONTROL_EXPORT Result requestConnection( QObject * receiver = 0, const char * member = 0 );

        /**
         * Activates or deactivates networking (as a whole).
         *
         * @param enabled true to activate networking, false otherwise
         */
        SOLIDCONTROL_EXPORT void releaseConnection();
    } // Networking

} // namespace Control
} // Solid

#endif
