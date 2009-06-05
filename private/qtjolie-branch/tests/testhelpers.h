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

#include <QtJolie/Fault>
#include <QtJolie/Message>
#include <QtJolie/Value>
#include "../qtjolie/sodephelpers_p.h"

inline void sodepDump(const Jolie::Value &value, int level)
{
    QByteArray indent;

    while (level>0) {
        indent+="    ";
        level--;
    }

    qDebug() << (indent+value.toByteArray()) << value.toInt() << value.toDouble();
    foreach (const QByteArray &name, value.childrenNames()) {
        QList<Jolie::Value> children = value.children(name);
        qDebug() << (indent+"Children:") << name;
        foreach (const Jolie::Value &child, children) {
            sodepDump(child, level+1);
        }
    }
}

inline void sodepDump(const Jolie::Message &message)
{
    qDebug() << "Resource :" << message.resourcePath();
    qDebug() << "Operation:" << message.operationName();
    qDebug() << "Fault    :" << message.fault().name();
    sodepDump(message.fault().data(), 1);
    qDebug() << "Value    :";
    sodepDump(message.data(), 1);
}

inline void sodepCompare(const Jolie::Value &v1, const Jolie::Value &v2)
{
    QCOMPARE(v1.isValid(), v2.isValid());

    QCOMPARE(v1.isByteArray(), v2.isByteArray());
    QCOMPARE(v1.isInt(), v2.isInt());
    QCOMPARE(v1.isDouble(), v2.isDouble());

    QCOMPARE(v1.toByteArray(), v2.toByteArray());
    QCOMPARE(v1.toInt(), v2.toInt());
    QCOMPARE(v1.toDouble(), v2.toDouble());

    QList<QByteArray> v1Names = v1.childrenNames();
    QList<QByteArray> v2Names = v2.childrenNames();
    QCOMPARE(v1Names, v2Names);

    foreach (const QByteArray &name, v1Names) {
        QList<Jolie::Value> v1Values = v1.children(name);
        QList<Jolie::Value> v2Values = v2.children(name);

        QCOMPARE(v1Values.size(), v2Values.size());

        for (int i=0; i<v1Values.size(); ++i) {
            sodepCompare(v1Values[i], v2Values[i]);
        }
    }
}

inline void sodepCompare(const Jolie::Fault &f1, const Jolie::Fault &f2)
{
    QCOMPARE(f1.isValid(), f2.isValid());
    QCOMPARE(f1.name(), f2.name());
    sodepCompare(f1.data(), f2.data());
}

inline void sodepCompare(const Jolie::Message &m1, const Jolie::Message &m2)
{
    QCOMPARE(m1.resourcePath(), m2.resourcePath());
    QCOMPARE(m1.operationName(), m2.operationName());
    sodepCompare(m1.fault(), m2.fault());
    sodepCompare(m1.data(), m2.data());
}

#endif
