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

#ifndef SODEPMESSAGE_H
#define SODEPMESSAGE_H

#include <sodepvalue.h>
#include <sodepfault.h>

class SodepMessagePrivate;

class Q_DECL_EXPORT SodepMessage
{
public:
    SodepMessage();
    explicit SodepMessage(const QString &resourcePath,
                          const QString &operationName,
                          qint64 id = 0);
    SodepMessage(const SodepMessage &other);
    ~SodepMessage();

    SodepMessage &operator=(const SodepMessage &other);

    qint64 id() const;

    QString resourcePath() const;
    QString operationName() const;

    SodepFault fault() const;
    void setFault(const SodepFault &fault);

    SodepValue data() const;
    void setData(const SodepValue &data);

    bool isValid();

    void writeTo(QIODevice &io) const;
    static SodepMessage readFrom(QIODevice &io);

private:
    SodepMessagePrivate * const d;
};

#endif
