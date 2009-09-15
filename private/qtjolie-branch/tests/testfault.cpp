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

#include <QtJolie/Fault>
#include "testhelpers.h"

using namespace Jolie;

class TestFault : public QObject
{
    Q_OBJECT

private slots:
    void shouldHandleInvalids()
    {
        Fault f;

        QCOMPARE(f.name(), QByteArray());
        QVERIFY(!f.data().isValid());

        QVERIFY(!f.isValid());
    }
    void shouldVerifyInitialState()
    {
        Fault f1("blup"), f2("blop", Value(42));

        QCOMPARE(f1.name(), QByteArray("blup"));
        QVERIFY(!f1.data().isValid());

        QCOMPARE(f2.name(), QByteArray("blop"));
        QVERIFY(f2.data().isValid());
        QCOMPARE(f2.data().toInt(), 42);

        f1 = f2;

        QCOMPARE(f1.name(), QByteArray("blop"));
        QVERIFY(f1.data().isValid());
        QCOMPARE(f1.data().toInt(), 42);
    }

    void shouldBeSerializable_data()
    {
        Value v(42);
        QByteArray vSerial = QByteArray::fromHex("020000002A00000000");

        QTest::addColumn<Fault>("original");
        QTest::addColumn<QByteArray>("serialized");

        QTest::newRow("empty fault") << Fault()
                                     << QByteArray::fromHex("00");
        QTest::newRow("no value fault") << Fault("foo")
                                        << QByteArray::fromHex("0100000003")+QByteArray("foo")
                                         + QByteArray::fromHex("0000000000");
        QTest::newRow("value fault") << Fault("bar", v)
                                     << QByteArray::fromHex("0100000003")+QByteArray("bar")
                                      + vSerial;
    }

    void shouldBeSerializable()
    {
        QBuffer buffer;

        QFETCH(Fault, original);
        QFETCH(QByteArray, serialized);
        Fault result;

        buffer.open(QIODevice::WriteOnly);
        sodepWrite(buffer, original);
        buffer.close();

        buffer.open(QIODevice::ReadOnly);
        result = sodepReadFault(buffer);
        buffer.close();

        sodepCompare(result, original);
        QCOMPARE(buffer.data(), serialized);
    }
};

QTEST_MAIN(TestFault)

#include "testfault.moc"
