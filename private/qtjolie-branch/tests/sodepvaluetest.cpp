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

#include <sodepvalue.h>

#include "sodeptesthelpers.h"

class SodepValueTest : public QObject
{
    Q_OBJECT

private slots:
    void shouldHandleInvalids()
    {
        SodepValue v;

        QCOMPARE(v.toInt(), 0);
        QCOMPARE(v.toDouble(), 0.0);
        QCOMPARE(v.toString(), QString());

        QVERIFY(!v.isValid());

        QVERIFY(!v.isString());
        QVERIFY(!v.isInt());
        QVERIFY(!v.isDouble());
    }

    void shouldRespectIntValues()
    {
        SodepValue v1(42), v2;

        QCOMPARE(v1.toInt(), 42);
        QCOMPARE(v2.toInt(), 0);

        QVERIFY(v1.isInt());
        QVERIFY(!v2.isInt());

        v2 = v1;

        QCOMPARE(v2.toInt(), 42);
        QVERIFY(v2.isInt());

        QCOMPARE(v2.toDouble(), 0.0);
        QCOMPARE(v2.toString(), QString());
    }

    void shouldRespectDoubleValues()
    {
        SodepValue v1(0.42), v2;

        QCOMPARE(v1.toDouble(), 0.42);
        QCOMPARE(v2.toDouble(), 0.0);

        QVERIFY(v1.isDouble());
        QVERIFY(!v2.isDouble());

        v2 = v1;

        QCOMPARE(v2.toDouble(), 0.42);
        QVERIFY(v2.isDouble());

        QCOMPARE(v2.toInt(), 0);
        QCOMPARE(v2.toString(), QString());
    }

    void shouldRespectStringValues()
    {
        SodepValue v1("42"), v2;

        QCOMPARE(v1.toString(), QString("42"));
        QCOMPARE(v2.toString(), QString());

        QVERIFY(v1.isString());
        QVERIFY(!v2.isString());

        v2 = v1;

        QCOMPARE(v2.toString(), QString("42"));
        QVERIFY(v2.isString());

        QCOMPARE(v2.toInt(), 0);
        QCOMPARE(v2.toDouble(), 0.0);
    }

    void shouldHandleChildren()
    {
        SodepValue v;

        v.children("first") << SodepValue(7) << SodepValue(8);
        v.children("second") << SodepValue(42);

        QCOMPARE(v.children("second").size(), 1);
        QCOMPARE(v.children("second")[0].toInt(), 42);

        QCOMPARE(v.children("first").size(), 2);
        QCOMPARE(v.children("first")[0].toInt(), 7);
        QCOMPARE(v.children("first")[1].toInt(), 8);

        QCOMPARE(v.children("bwaaaaah!").size(), 0);
    }

    void shouldBeSerializable_data()
    {
        QTest::addColumn<SodepValue>("original");
        QTest::addColumn<QByteArray>("serialized");

        QTest::newRow("empty value") << SodepValue()
                                      << QByteArray::fromHex("0000000000");
        QTest::newRow("double value") << SodepValue(0.42)
                                      << QByteArray::fromHex("033FDAE147AE147AE100000000");
        QTest::newRow("int value") << SodepValue(42)
                                   << QByteArray::fromHex("020000002A00000000");
        QTest::newRow("string value") << SodepValue("foo")
                                      << QByteArray::fromHex("0100000003")+QByteArray("foo")
                                       + QByteArray::fromHex("00000000");

        SodepValue complex("complex");
        complex.children("foo") << SodepValue(42);
        complex.children("bar") << SodepValue(0.42);
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

        QFETCH(SodepValue, original);
        QFETCH(QByteArray, serialized);
        SodepValue result;

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

QTEST_MAIN(SodepValueTest)

#include "sodepvaluetest.moc"
