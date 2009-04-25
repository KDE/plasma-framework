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

#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include "sodephelpers_p.h"

namespace Jolie
{

class ValuePrivate
{
public:
    QVariant content;
    QMap<QString, QList<Value> > children;
};

} // namespace Jolie

using namespace Jolie;

Value::Value()
    : d(new ValuePrivate)
{
}

Value::Value(const QString &content)
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

QStringList Value::childrenNames() const
{
    return d->children.keys();
}

QList<Value> & Value::children(const QString &name)
{
    return d->children[name];
}

const QList<Value> & Value::children(const QString &name) const
{
    return d->children[name];
}

QString Value::toString() const
{
    if (isString()) {
        return d->content.toString();
    } else {
        return QString();
    }
}

qint32 Value::toInt() const
{
    if (isInt()) {
        return d->content.toInt();
    } else {
        return 0;
    }
}

double Value::toDouble() const
{
    if (isDouble()) {
        return d->content.toDouble();
    } else {
        return 0.0;
    }
}

bool Value::isString() const
{
    return d->content.type()==QVariant::String;
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
    return isString() || isInt() || isDouble();
}

