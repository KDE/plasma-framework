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

#include "value.h"

#include <QtCore/QVariant>

#include "sodephelpers_p.h"

namespace Jolie
{

class ValuePrivate
{
public:
    QVariant content;
    QMap<QByteArray, Value::List> children;
};

} // namespace Jolie

using namespace Jolie;

static int _qtjolie_valueTypeId = qRegisterMetaType<Value>();

Value::Value()
    : d(new ValuePrivate)
{
}

Value::Value(const QByteArray &content)
    : d(new ValuePrivate)
{
    d->content = content;
}

Value::Value(qint32 content)
    : d(new ValuePrivate)
{
    d->content = content;
}

Value::Value(double content)
    : d(new ValuePrivate)
{
    d->content = content;
}

Value::Value(const Value &other)
    : d(new ValuePrivate)
{
    *d = *other.d;
}

Value::~Value()
{
    delete d;
}

Value &Value::operator=(const Value &other)
{
    *d = *other.d;

    return *this;
}

QList<QByteArray> Value::childrenNames() const
{
    return d->children.keys();
}

Value::List &Value::children(const QByteArray &name)
{
    return d->children[name];
}

const Value::List &Value::children(const QByteArray &name) const
{
    return d->children[name];
}

QByteArray Value::toByteArray() const
{
    if (isByteArray()) {
        return d->content.toByteArray();
    } else {
        return QByteArray();
    }
}

qint32 Value::toInt() const
{
    if (isInt()) {
        return d->content.toInt();
    } else if (isByteArray()) {
        return d->content.toByteArray().toInt();
    } else {
        return 0;
    }
}

double Value::toDouble() const
{
    if (isDouble()) {
        return d->content.toDouble();
    } else if (isByteArray()) {
        return d->content.toByteArray().toInt();
    } else {
        return 0.0;
    }
}

bool Value::isByteArray() const
{
    return d->content.type()==QVariant::ByteArray;
}

bool Value::isInt() const
{
    return d->content.type()==QVariant::Int;
}

bool Value::isDouble() const
{
    return d->content.type()==QVariant::Double;
}

bool Value::isValid() const
{
    return isByteArray() || isInt() || isDouble();
}

