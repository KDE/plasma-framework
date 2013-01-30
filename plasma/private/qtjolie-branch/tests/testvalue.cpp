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

#include <QtCore/QObject>
#include <QtTest/QtTest>

#include <QtJolie/Value>

#include "testhelpers.h"

using namespace Jolie;

class TestValue : public QObject
{
    Q_OBJECT

private slots:
    void shouldHandleInvalids()
    {
        Value v;

        QCOMPARE(v.toInt(), 0);
        QCOMPARE(v.toDouble(), 0.0);
        QCOMPARE(v.toByteArray(), QByteArray());

        QVERIFY(!v.isValid());

        QVERIFY(!v.isByteArray());
        QVERIFY(!v.isInt());
        QVERIFY(!v.isDouble());
    }

    void shouldRespectIntValues()
    {
        Value v1(42), v2;

        QCOMPARE(v1.toInt(), 42);
        QCOMPARE(v2.toInt(), 0);

        QVERIFY(v1.isInt());
        QVERIFY(!v2.isInt());

        v2 = v1;

        QCOMPARE(v2.toInt(), 42);
        QVERIFY(v2.isInt());

        QCOMPARE(v2.toDouble(), 0.0);
        QCOMPARE(v2.toByteArray(), QByteArray());
    }

    void shouldRespectDoubleValues()
    {
        Value v1(0.42), v2;

        QCOMPARE(v1.toDouble(), 0.42);
        QCOMPARE(v2.toDouble(), 0.0);

        QVERIFY(v1.isDouble());
        QVERIFY(!v2.isDouble());

        v2 = v1;

        QCOMPARE(v2.toDouble(), 0.42);
        QVERIFY(v2.isDouble());

        QCOMPARE(v2.toInt(), 0);
        QCOMPARE(v2.toByteArray(), QByteArray());
    }

    void shouldAutoConvertByteArrayValues()
    {
        Value v1("42"), v2;

        QCOMPARE(v1.toByteArray(), QByteArray("42"));
        QCOMPARE(v2.toByteArray(), QByteArray());

        QVERIFY(v1.isByteArray());
        QVERIFY(!v2.isByteArray());

        v2 = v1;

        QCOMPARE(v2.toByteArray(), QByteArray("42"));
        QVERIFY(v2.isByteArray());

        QCOMPARE(v2.toInt(), 42);
        QCOMPARE(v2.toDouble(), 42.0);
    }

    void shouldHandleChildren()
    {
        Value v;

        v.children("first") << Value(7) << Value(8);
        v.children("second") << Value(42);

        QCOMPARE(v.children("second").size(), 1);
        QCOMPARE(v.children("second")[0].toInt(), 42);

        QCOMPARE(v.children("first").size(), 2);
        QCOMPARE(v.children("first")[0].toInt(), 7);
        QCOMPARE(v.children("first")[1].toInt(), 8);

        QCOMPARE(v.children("bwaaaaah!").size(), 0);
    }

    void shouldBeSerializable_data()
    {
        QTest::addColumn<Value>("original");
        QTest::addColumn<QByteArray>("serialized");

        QTest::newRow("empty value") << Value()
                                      << QByteArray::fromHex("0000000000");
        QTest::newRow("double value") << Value(0.42)
                                      << QByteArray::fromHex("033FDAE147AE147AE100000000");
        QTest::newRow("int value") << Value(42)
                                   << QByteArray::fromHex("020000002A00000000");
        QTest::newRow("string value") << Value("foo")
                                      << QByteArray::fromHex("0100000003")+QByteArray("foo")
                                       + QByteArray::fromHex("00000000");

        Value complex("complex");
        complex.children("foo") << Value(42);
        complex.children("bar") << Value(0.42);
        QTest::newRow("complex value") << complex
                                       << QByteArray::fromHex("0100000007")+QByteArray("complex")
                                        + QByteArray::fromHex("00000002") // two children
                                        + QByteArray::fromHex("00000003")+QByteArray("bar")
                                        + QByteArray::fromHex("00000001") // one value
                                        + QByteArray::fromHex("033FDAE147AE147AE100000000")
                                        + QByteArray::fromHex("00000003")+QByteArray("foo")
                                        + QByteArray::fromHex("00000001") // one value
                                        + QByteArray::fromHex("020000002A00000000");
    }

    void shouldBeSerializable()
    {
        QBuffer buffer;

        QFETCH(Value, original);
        QFETCH(QByteArray, serialized);
        Value result;

        buffer.open(QIODevice::WriteOnly);
        sodepWrite(buffer, original);
        buffer.close();

        buffer.open(QIODevice::ReadOnly);
        result = sodepReadValue(buffer);
        buffer.close();

        sodepCompare(result, original);
        QCOMPARE(buffer.data(), serialized);
    }
};

QTEST_MAIN(TestValue)

#include "testvalue.moc"
