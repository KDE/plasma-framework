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

#include "clientpinrequest.h"

#include "service.h"
#include "private/remoteservice_p.h"

#include <kdebug.h>
#include <klocalizedstring.h>

namespace Plasma
{

class ClientPinRequestPrivate {
public:
    ClientPinRequestPrivate(RemoteService *service)
        : service(service)
    {
    }

    ~ClientPinRequestPrivate() {}

    RemoteService *service;
    QString pin;
};

ClientPinRequest::ClientPinRequest(RemoteService *service)
    : QObject(service),
      d(new ClientPinRequestPrivate(service))
{
}

ClientPinRequest::~ClientPinRequest()
{
    delete d;
}

QString ClientPinRequest::description() const
{
    return i18n("You have requested access to the %1 hosted at %2.", d->service->name(),
                                                                     d->service->location());
}

void ClientPinRequest::setPin(const QString &pin)
{
    kDebug() << "pin = " << pin;
    d->pin = pin;
    emit changed(this);
}

QString ClientPinRequest::pin() const
{
    return d->pin;
}

} // Plasma namespace

#include "clientpinrequest.moc"

