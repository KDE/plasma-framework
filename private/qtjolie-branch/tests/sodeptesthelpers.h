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

#ifndef SODEPTESTHELPERS_H
#define SODEPTESTHELPERS_H

#include <QtTest/QtTest>

#include <sodepfault.h>
#include <sodepmessage.h>
#include <sodepvalue.h>
#include <sodephelpers_p.h>

Q_DECLARE_METATYPE(SodepValue);

inline void sodepCompare(const SodepValue &v1, const SodepValue &v2)
{
    QCOMPARE(v1.isValid(), v2.isValid());

    QCOMPARE(v1.isString(), v2.isString());
    QCOMPARE(v1.isInt(), v2.isInt());
    QCOMPARE(v1.isDouble(), v2.isDouble());

    QCOMPARE(v1.toString(), v2.toString());
    QCOMPARE(v1.toInt(), v2.toInt());
    QCOMPARE(v1.toDouble(), v2.toDouble());

    QStringList v1Names = v1.childrenNames();
    QStringList v2Names = v2.childrenNames();
    QCOMPARE(v1Names, v2Names);

    foreach (const QString &name, v1Names) {
        QList<SodepValue> v1Values = v1.children(name);
        QList<SodepValue> v2Values = v2.children(name);

        QCOMPARE(v1Values.size(), v2Values.size());

        for (int i=0; i<v1Values.size(); ++i) {
            sodepCompare(v1Values[i], v2Values[i]);
        }
    }
}

Q_DECLARE_METATYPE(SodepFault);

inline void sodepCompare(const SodepFault &f1, const SodepFault &f2)
{
    QCOMPARE(f1.isValid(), f2.isValid());
    QCOMPARE(f1.name(), f2.name());
    sodepCompare(f1.data(), f2.data());
}

Q_DECLARE_METATYPE(SodepMessage);

inline void sodepCompare(const SodepMessage &m1, const SodepMessage &m2)
{
    QCOMPARE(m1.resourcePath(), m2.resourcePath());
    QCOMPARE(m1.operationName(), m2.operationName());
    sodepCompare(m1.fault(), m2.fault());
    sodepCompare(m1.data(), m2.data());
}

#endif
