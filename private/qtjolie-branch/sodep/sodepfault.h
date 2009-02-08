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

#ifndef SODEPFAULT_H
#define SODEPFAULT_H

#include <sodepvalue.h>

class SodepFaultPrivate;

class Q_DECL_EXPORT SodepFault
{
public:
    SodepFault();
    explicit SodepFault(const QString &name, const SodepValue &data = SodepValue());

    SodepFault(const SodepFault &other);

    ~SodepFault();

    SodepFault &operator=(const SodepFault &other);

    QString name() const;
    SodepValue data() const;

    bool isValid() const;

private:
    SodepFaultPrivate * const d;
};

#endif
