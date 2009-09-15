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

#include "message.h"

#include <QtCore/QByteArray>

#include "sodephelpers_p.h"

namespace Jolie
{

class MessagePrivate
{
public:
    MessagePrivate() : id(0) {}

    qint64 id;
    QByteArray resourcePath;
    QByteArray operationName;
    Fault fault;
    Value data;
};

} // namespace Jolie

using namespace Jolie;

static int _qtjolie_messageTypeId = qRegisterMetaType<Message>();

Message::Message()
    : d(new MessagePrivate)
{
}

Message::Message(const QByteArray &resourcePath, const QByteArray &operationName, qint64 id)
    : d(new MessagePrivate)
{
    static qint64 lastId = 0;

    if (id==0) {
        d->id = ++lastId;
    } else {
        d->id = id;
    }
    d->resourcePath = resourcePath;
    d->operationName = operationName;
}

Message::Message(const Message &other)
    : d(new MessagePrivate)
{
    *d = *other.d;
}

Message::~Message()
{
    delete d;
}

Message &Message::operator=(const Message &other)
{
    *d = *other.d;

    return *this;
}

qint64 Message::id() const
{
    return d->id;
}

QByteArray Message::resourcePath() const
{
    return d->resourcePath;
}

QByteArray Message::operationName() const
{
    return d->operationName;
}

Fault Message::fault() const
{
    return d->fault;
}

void Message::setFault(const Fault &fault)
{
    d->fault = fault;
}

Value Message::data() const
{
    return d->data;
}

void Message::setData(const Value &data)
{
    d->data = data;
}

bool Message::isValid()
{
    return !d->resourcePath.isEmpty() && !d->operationName.isEmpty();
}

