/*
 * Copyright 2009 by Rob Scheepmaker <r.scheepmaker@student.utwente.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA  02110-1301  USA
 */

#ifndef PLASMA_CLIENTPINREQUEST_H
#define PLASMA_CLIENTPINREQUEST_H

#include "plasma_export.h"

#include <QtCore/QObject>

class QString;
class KUrl;

namespace Plasma
{

class ClientPinRequestPrivate;
class RemoteService;
class Service;

/**
 * @class ClientPinRequest plasma/clientpinrequest.h <Plasma/ClientPinRequest>
 *
 * describes an outgoing connection. this is just passed to AuthorizationInterface when a remote
 * widget asks to do pin pairing first, so the shell can ask the user for a pin.
 *
 * @since 4.4
 */
class PLASMA_EXPORT ClientPinRequest : public QObject
{
    Q_OBJECT
    public:
        /**
         * @returns nice i18n'ed description of this outgoing connection.
         */
        QString description() const;

        /**
         * @param pin set a pin for pin pairing.
         */
        void setPin(const QString &pin);

        /**
         * @returns the pin for pin pairing.
         */
        QString pin() const;

    Q_SIGNALS:
        void changed(Plasma::ClientPinRequest *);

    private:
        ClientPinRequest();
        ClientPinRequest(RemoteService *service);
        ~ClientPinRequest();

        ClientPinRequestPrivate * const d;

        friend class RemoteService;

};
} // Plasma namespace

#endif

