/*
Copyright 2008 Will Stephenson <wstephenson@kde.org>

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of
the License or (at your option) version 3 or any later version
accepted by the membership of KDE e.V. (or its successor approved
by the membership of KDE e.V.), which shall act as a proxy 
defined in Section 14 of version 3 of the license.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "networkipv4config.h"

namespace Solid
{
namespace Control
{
class IPv4Config::Private
{
public:
    Private(const QList<IPv4Address> &theAddresses,
        const QList<quint32> &theNameservers,
        const QStringList &theDomains, const QList<IPv4Route> &theRoutes)
        : addresses(theAddresses), nameservers(theNameservers),
        domains(theDomains), routes(theRoutes)
    {}
    Private()
    {}
    QList<IPv4Address> addresses;
    QList<quint32> nameservers;
    QStringList domains;
    QList<IPv4Route> routes;
};


class IPv4Address::Private
{
public:
    Private(quint32 theAddress, quint32 theNetMask, quint32 theGateway)
        : address(theAddress), netMask(theNetMask), gateway(theGateway)
    {}
    Private()
        : address(0), netMask(0), gateway(0)
    {}
    quint32 address;
    quint32 netMask;
    quint32 gateway;
};

class IPv4Route::Private
{
public:
    Private(quint32 theRoute, quint32 thePrefix, quint32 theNextHop, quint32 theMetric)
        : route(theRoute), prefix(thePrefix), nextHop(theNextHop), metric(theMetric)
    {}
    Private()
        : route(0), prefix(0), nextHop(0), metric(0)
    {}
    quint32 route;
    quint32 prefix;
    quint32 nextHop;
    quint32 metric;
};
}
}

Solid::Control::IPv4Address::IPv4Address(quint32 address, quint32 netMask, quint32 gateway)
: d(new Private(address, netMask, gateway))
{
}

Solid::Control::IPv4Address::IPv4Address()
: d(new Private())
{
}

Solid::Control::IPv4Address::~IPv4Address()
{
    delete d;
}

Solid::Control::IPv4Address::IPv4Address(const IPv4Address &other)
: d(new Private(*other.d))
{
}

quint32 Solid::Control::IPv4Address::address() const
{
    return d->address;
}

quint32 Solid::Control::IPv4Address::netMask() const
{
    return d->netMask;
}

quint32 Solid::Control::IPv4Address::gateway() const
{
    return d->gateway;
}

Solid::Control::IPv4Address &Solid::Control::IPv4Address::operator=(const Solid::Control::IPv4Address &other)
{
    if (this == &other)
        return *this;

    *d = *other.d;
    return *this;
}

bool Solid::Control::IPv4Address::isValid() const
{
    return d->address != 0;
}

Solid::Control::IPv4Route::IPv4Route(quint32 route, quint32 prefix, quint32 nextHop, quint32 metric)
: d(new Private(route, prefix, nextHop, metric))
{
}

Solid::Control::IPv4Route::IPv4Route()
: d(new Private())
{
}

Solid::Control::IPv4Route::~IPv4Route()
{
    delete d;
}

Solid::Control::IPv4Route::IPv4Route(const IPv4Route &other)
: d(new Private(*other.d))
{
}

quint32 Solid::Control::IPv4Route::route() const
{
    return d->route;
}

quint32 Solid::Control::IPv4Route::prefix() const
{
    return d->prefix;
}

quint32 Solid::Control::IPv4Route::nextHop() const
{
    return d->nextHop;
}

quint32 Solid::Control::IPv4Route::metric() const
{
    return d->metric;
}

Solid::Control::IPv4Route &Solid::Control::IPv4Route::operator=(const Solid::Control::IPv4Route &other)
{
    if (this == &other)
        return *this;

    *d = *other.d;
    return *this;
}

bool Solid::Control::IPv4Route::isValid() const
{
    return d->route != 0;
}


Solid::Control::IPv4Config::IPv4Config(const QList<IPv4Address> &addresses,
        const QList<quint32> &nameservers,
        const QStringList &domains,
        const QList<IPv4Route> &routes)
: d(new Private(addresses, nameservers, domains, routes))
{
}

Solid::Control::IPv4Config::IPv4Config()
: d(new Private())
{
}

Solid::Control::IPv4Config::IPv4Config(const Solid::Control::IPv4Config& other)
{
    d = new Private(*other.d);
}

Solid::Control::IPv4Config::~IPv4Config()
{
    delete d;
}

QList<Solid::Control::IPv4Address> Solid::Control::IPv4Config::addresses() const
{
    return d->addresses;
}

QList<quint32> Solid::Control::IPv4Config::nameservers() const
{
    return d->nameservers;
}

QStringList Solid::Control::IPv4Config::domains() const
{
    return d->domains;
}

Solid::Control::IPv4Config &Solid::Control::IPv4Config::operator=(const Solid::Control::IPv4Config& other)
{
    if (this == &other)
        return *this;

    *d = *other.d;
    return *this;
}

bool Solid::Control::IPv4Config::isValid() const
{
    return !d->addresses.isEmpty();
}

