/**
  * This file is part of the KDE project
  * Copyright (C) 2008 Kevin Ottens <ervin@kde.org>
  *
  * This library is free software; you can redistribute it and/or
  * modify it under the terms of the GNU Library General Public
  * License as published by the Free Software Foundation; either
  * version 2 of the License, or (at your option) any later version.
  *
  * This library is distributed in the hope that it will be useful,
  * but WITHOUT ANY WARRANTY; without even the implied warranty of
  * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  * Library General Public License for more details.
  *
  * You should have received a copy of the GNU Library General Public License
  * along with this library; see the file COPYING.LIB.  If not, write to
  * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  * Boston, MA 02110-1301, USA.
  */

#include "fault.h"

#include <QtCore/QByteArray>

#include "sodephelpers_p.h"

namespace Jolie
{

class FaultPrivate
{
public:
    QByteArray name;
    Value data;
};

} // namespace Jolie

using namespace Jolie;

static int _qtjolie_faultTypeId = qRegisterMetaType<Fault>();

Fault::Fault()
    : d(new FaultPrivate)
{

}

Fault::Fault(const QByteArray &name, const Value &data)
    : d(new FaultPrivate)
{
    d->name = name;
    d->data = data;
}

Fault::Fault(const Fault &other)
    : d(new FaultPrivate)
{
    *d = *other.d;
}

Fault::~Fault()
{
    delete d;
}

Fault &Fault::operator=(const Fault &other)
{
    *d = *other.d;

    return *this;
}

QByteArray Fault::name() const
{
    return d->name;
}

Value Fault::data() const
{
    return d->data;
}

bool Fault::isValid() const
{
    return !d->name.isEmpty();
}

