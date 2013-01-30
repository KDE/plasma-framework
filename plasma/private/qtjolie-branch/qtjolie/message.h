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

#ifndef QTJOLIE_MESSAGE_H
#define QTJOLIE_MESSAGE_H

#include <qtjolie/value.h>
#include <qtjolie/fault.h>

#include <QtCore/QMetaType>

namespace Jolie
{
class MessagePrivate;

class Q_DECL_EXPORT Message
{
public:
    Message();
    explicit Message(const QByteArray &resourcePath,
                     const QByteArray &operationName,
                     qint64 id = 0);
    Message(const Message &other);
    ~Message();

    Message &operator=(const Message &other);

    qint64 id() const;

    QByteArray resourcePath() const;
    QByteArray operationName() const;

    Fault fault() const;
    void setFault(const Fault &fault);

    Value data() const;
    void setData(const Value &data);

    bool isValid();

private:
    MessagePrivate * const d;
};

} // namespace Jolie

Q_DECLARE_METATYPE(Jolie::Message)

#endif

