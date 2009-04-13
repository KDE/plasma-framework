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

#ifndef SODEPPENDINGCALL_H
#define SODEPPENDINGCALL_H

#include <QtCore/QExplicitlySharedDataPointer>

class SodepClient;
class SodepPendingCallPrivate;
class SodepMessage;

class Q_DECL_EXPORT SodepPendingCall
{
public:
    SodepPendingCall(const SodepPendingCall &other);
    ~SodepPendingCall();

    SodepPendingCall &operator=(const SodepPendingCall &other);

    bool isFinished() const;
    SodepMessage reply() const;

    void waitForFinished();

private:
    friend class SodepClient;

    SodepPendingCall(); // Not defined
    SodepPendingCall(QExplicitlySharedDataPointer<SodepPendingCallPrivate> dd);

    QExplicitlySharedDataPointer<SodepPendingCallPrivate> d;
};

#endif
