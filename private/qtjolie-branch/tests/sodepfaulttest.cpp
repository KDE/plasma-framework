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

#include <sodepfault.h>
#include "sodeptesthelpers.h"

class SodepFaultTest : public QObject
{
    Q_OBJECT

private slots:
    void shouldHandleInvalids()
    {
        SodepFault f;

        QCOMPARE(f.name(), QString());
        QVERIFY(!f.data().isValid());

        QVERIFY(!f.isValid());
    }
    void shouldVerifyInitialState()
    {
        SodepFault f1("blup"), f2("blop", SodepValue(42));

        QCOMPARE(f1.name(), QString("blup"));
        QVERIFY(!f1.data().isValid());

        QCOMPARE(f2.name(), QString("blop"));
        QVERIFY(f2.data().isValid());
        QCOMPARE(f2.data().toInt(), 42);

        f1 = f2;

        QCOMPARE(f1.name(), QString("blop"));
        QVERIFY(f1.data().isValid());
        QCOMPARE(f1.data().toInt(), 42);
    }

    void shouldBeSerializable_data()
    {
        SodepValue v(42);
        QByteArray vSerial = QByteArray::fromHex("020000002A00000000");

        QTest::addColumn<SodepFault>("original");
        QTest::addColumn<QByteArray>("serialized");

        QTest::newRow("empty fault") << SodepFault()
                                     << QByteArray::fromHex("00");
        QTest::newRow("no value fault") << SodepFault("foo")
                                        << QByteArray::fromHex("0100000003")+QByteArray("foo")
                                         + QByteArray::fromHex("0000000000");
        QTest::newRow("value fault") << SodepFault("bar", v)
                                     << QByteArray::fromHex("0100000003")+QByteArray("bar")
                                      + vSerial;
    }

    void shouldBeSerializable()
    {
        QBuffer buffer;

        QFETCH(SodepFault, original);
        QFETCH(QByteArray, serialized);
        SodepFault result;

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

QTEST_MAIN(SodepFaultTest)

#include "sodepfaulttest.moc"
