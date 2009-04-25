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

#include "sodepvalue.h"

#include <QtCore/QStringList>
#include <QtCore/QVariant>

#include "sodephelpers_p.h"

class SodepValuePrivate
{
public:
    QVariant content;
    QMap<QString, QList<SodepValue> > children;
};

SodepValue::SodepValue()
    : d(new SodepValuePrivate)
{
}

SodepValue::SodepValue(const QString &content)
    : d(new SodepValuePrivate)
{
    d->content = content;
}

SodepValue::SodepValue(qint32 content)
    : d(new SodepValuePrivate)
{
    d->content = content;
}

SodepValue::SodepValue(double content)
    : d(new SodepValuePrivate)
{
    d->content = content;
}

SodepValue::SodepValue(const SodepValue &other)
    : d(new SodepValuePrivate)
{
    *d = *other.d;
}

SodepValue::~SodepValue()
{
    delete d;
}

SodepValue &SodepValue::operator=(const SodepValue &other)
{
    *d = *other.d;

    return *this;
}

QStringList SodepValue::childrenNames() const
{
    return d->children.keys();
}

QList<SodepValue> & SodepValue::children(const QString &name)
{
    return d->children[name];
}

const QList<SodepValue> & SodepValue::children(const QString &name) const
{
    return d->children[name];
}

QString SodepValue::toString() const
{
    if (isString()) {
        return d->content.toString();
    } else {
        return QString();
    }
}

qint32 SodepValue::toInt() const
{
    if (isInt()) {
        return d->content.toInt();
    } else {
        return 0;
    }
}

double SodepValue::toDouble() const
{
    if (isDouble()) {
        return d->content.toDouble();
    } else {
        return 0.0;
    }
}

bool SodepValue::isString() const
{
    return d->content.type()==QVariant::String;
}

bool SodepValue::isInt() const
{
    return d->content.type()==QVariant::Int;
}

bool SodepValue::isDouble() const
{
    return d->content.type()==QVariant::Double;
}

bool SodepValue::isValid() const
{
    return isString() || isInt() || isDouble();
}
