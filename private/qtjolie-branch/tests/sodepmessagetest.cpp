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

#include <sodepmessage.h>
#include "sodeptesthelpers.h"

class SodepMessageTest : public QObject
{
    Q_OBJECT

private slots:
    void shouldVerifyInitialState()
    {
        SodepMessage m1("/foo", "bar");
        SodepMessage m2("/pata/pata", "pon", 2);

        QCOMPARE(m1.resourcePath(), QString("/foo"));
        QCOMPARE(m1.operationName(), QString("bar"));
        QCOMPARE(m1.id(), qint64(0));

        QCOMPARE(m2.resourcePath(), QString("/pata/pata"));
        QCOMPARE(m2.operationName(), QString("pon"));
        QCOMPARE(m2.id(), qint64(2));

        m1 = m2;

        QCOMPARE(m1.resourcePath(), QString("/pata/pata"));
        QCOMPARE(m1.operationName(), QString("pon"));
        QCOMPARE(m2.id(), qint64(2));
    }

    void shouldBeSerializable_data()
    {
        SodepValue v(42);
        QByteArray vSerial = QByteArray::fromHex("020000002A00000000");
        SodepFault f("foo");
        QByteArray fSerial = QByteArray::fromHex("0100000003")+QByteArray("foo")
                           + QByteArray::fromHex("0000000000");

        QTest::addColumn<SodepMessage>("original");
        QTest::addColumn<QByteArray>("serialized");

        QTest::newRow("no payload message") << SodepMessage("/pata", "pon")
                                            << QByteArray::fromHex("0000000000000000")
                                             + QByteArray::fromHex("00000005")+QByteArray("/pata")
                                             + QByteArray::fromHex("00000003")+QByteArray("pon")
                                             + QByteArray::fromHex("00")
                                             + QByteArray::fromHex("0000000000");

        SodepMessage payload("/pata", "pon");
        payload.setFault(f);
        payload.setData(v);
        QTest::newRow("payload message") << payload
                                         << QByteArray::fromHex("0000000000000000")
                                          + QByteArray::fromHex("00000005")+QByteArray("/pata")
                                          + QByteArray::fromHex("00000003")+QByteArray("pon")
                                          + fSerial
                                          + vSerial;

        SodepMessage payloadId("/pata", "pon", 42);
        payloadId.setFault(f);
        payloadId.setData(v);
        QTest::newRow("payload and id message") << payloadId
                                                << QByteArray::fromHex("000000000000002A")
                                                 + QByteArray::fromHex("00000005")+QByteArray("/pata")
                                                 + QByteArray::fromHex("00000003")+QByteArray("pon")
                                                 + fSerial
                                                 + vSerial;
    }

    void shouldBeSerializable()
    {
        QBuffer buffer;

        QFETCH(SodepMessage, original);
        QFETCH(QByteArray, serialized);
        SodepMessage result;

        buffer.open(QIODevice::WriteOnly);
        original.writeTo(buffer);
        buffer.close();

        buffer.open(QIODevice::ReadOnly);
        result = SodepMessage::readFrom(buffer);
        buffer.close();

        sodepCompare(result, original);
        QCOMPARE(buffer.data(), serialized);
    }
};

QTEST_MAIN(SodepMessageTest)

#include "sodepmessagetest.moc"
