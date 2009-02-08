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

#ifndef SODEPVALUE_H
#define SODEPVALUE_H

#include <QtCore/QIODevice>
#include <QtCore/QList>

class SodepValuePrivate;

class Q_DECL_EXPORT SodepValue
{
public:
    SodepValue();

    explicit SodepValue(const QString &content);
    explicit SodepValue(qint32 content);
    explicit SodepValue(double content);

    SodepValue(const SodepValue &other);

    ~SodepValue();

    SodepValue &operator=(const SodepValue &other);

    QStringList childrenNames() const;
    QList<SodepValue> &children(const QString &name);
    const QList<SodepValue> &children(const QString &name) const;

    QString toString() const;
    qint32 toInt() const;
    double toDouble() const;

    bool isString() const;
    bool isInt() const;
    bool isDouble() const;

    bool isValid() const;

private:
    SodepValuePrivate * const d;
};

#endif
