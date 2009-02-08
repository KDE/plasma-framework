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

#include "sodepfault.h"

#include <QtCore/QString>

#include "sodephelpers_p.h"

class SodepFaultPrivate
{
public:
    QString name;
    SodepValue data;
};

SodepFault::SodepFault()
    : d(new SodepFaultPrivate)
{

}

SodepFault::SodepFault(const QString &name, const SodepValue &data)
    : d(new SodepFaultPrivate)
{
    d->name = name;
    d->data = data;
}

SodepFault::SodepFault(const SodepFault &other)
    : d(new SodepFaultPrivate)
{
    *d = *other.d;
}

SodepFault::~SodepFault()
{
    delete d;
}

SodepFault &SodepFault::operator=(const SodepFault &other)
{
    *d = *other.d;

    return *this;
}

QString SodepFault::name() const
{
    return d->name;
}

SodepValue SodepFault::data() const
{
    return d->data;
}

bool SodepFault::isValid() const
{
    return !d->name.isEmpty();
}

