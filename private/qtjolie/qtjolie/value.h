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

#ifndef QTJOLIE_VALUE_H
#define QTJOLIE_VALUE_H

#include <QtCore/QIODevice>
#include <QtCore/QList>
#include <QtCore/QMetaType>

namespace Jolie
{
class ValuePrivate;

class Q_DECL_EXPORT Value
{
public:
    typedef QList<Value> List;

    Value();

    explicit Value(const QByteArray &content);
    explicit Value(qint32 content);
    explicit Value(double content);

    Value(const Value &other);

    ~Value();

    Value &operator=(const Value &other);

    QList<QByteArray> childrenNames() const;
    Value::List &children(const QByteArray &name);
    const Value::List &children(const QByteArray &name) const;

    QByteArray toByteArray() const;
    qint32 toInt() const;
    double toDouble() const;

    bool isByteArray() const;
    bool isInt() const;
    bool isDouble() const;

    bool isValid() const;

private:
    ValuePrivate * const d;
};

} // namespace Jolie

Q_DECLARE_METATYPE(Jolie::Value)

#endif

