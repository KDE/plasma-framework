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

#ifndef QTJOLIE_FAULT_H
#define QTJOLIE_FAULT_H

#include <qtjolie/value.h>

#include <QtCore/QMetaType>

namespace Jolie
{
class FaultPrivate;

class Q_DECL_EXPORT Fault
{
public:
    Fault();
    explicit Fault(const QByteArray &name, const Value &data = Value());

    Fault(const Fault &other);

    ~Fault();

    Fault &operator=(const Fault &other);

    QByteArray name() const;
    Value data() const;

    bool isValid() const;

private:
    FaultPrivate * const d;
};

} // namespace Jolie

Q_DECLARE_METATYPE(Jolie::Fault)

#endif

