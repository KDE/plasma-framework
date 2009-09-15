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
#include <QtCore/QProcess>
#include <QtTest/QtTest>

#include <QtJolie/Client>
#include <QtJolie/Message>
#include <QtJolie/MetaService>
#include <QtJolie/PendingCall>
#include "testhelpers.h"

#ifndef DATA_DIR
    #error "DATA_DIR is not set. A directory containing test jolie scripts is required for this test"
#endif

using namespace Jolie;

class TestMetaService : public QObject
{
    Q_OBJECT

    MetaService m_metaService;
    Client *m_client;

public:
    TestMetaService()
        : QObject()
    {
    }

private slots:
    void initTestCase()
    {
        QVERIFY2(m_metaService.start(), "Looks like you don't have Jolie's metaservice command");
        QTest::qWait(1000);

        m_client = new Client(QString::fromUtf8("localhost"), 9000);
    }

    void cleanupTestCase()
    {
        delete m_client;
        QVERIFY(m_metaService.stop());
    }

    void shouldLoadService_data()
    {
        QTest::addColumn<QString>("resourcePrefix");
        QTest::addColumn<QString>("fileName");

        QTest::newRow("printer service") << QString::fromUtf8("Printer") << QString::fromUtf8("printer.ol");
        QTest::newRow("math service") << QString::fromUtf8("Math") << QString::fromUtf8("math.ol");
    }

    void shouldLoadService()
    {
        QFETCH(QString, resourcePrefix);
        QFETCH(QString, fileName);

        QCOMPARE(m_metaService.loadService(resourcePrefix, QString::fromUtf8(DATA_DIR"/")+fileName),
                 resourcePrefix);
    }

    void shouldListServices()
    {
        QStringList expected;
        expected << QString::fromUtf8("Math") << QString::fromUtf8("Printer");
        QCOMPARE(m_metaService.loadedServices(), expected);
    }

    void shouldPlaceServiceCalls_data()
    {
        QTest::addColumn<QByteArray>("path");
        QTest::addColumn<QByteArray>("method");
        QTest::addColumn<Value>("data");
        QTest::addColumn<Value>("replyData");

        QTest::newRow("printer service") << QByteArray("/Printer") << QByteArray("printInput") << Value("Patapatapon!") << Value("success");
        QTest::newRow("math service") << QByteArray("/Math") << QByteArray("twice") << Value(10.5) << Value(21.0);
    }

    void shouldPlaceServiceCalls()
    {
        QFETCH(QByteArray, path);
        QFETCH(QByteArray, method);
        QFETCH(Value, data);
        QFETCH(Value, replyData);

        Message message(path, method);
        message.setData(data);

        Message reply = m_client->call(message);

        Message expected("/", method);
        expected.setData(replyData);

        sodepCompare(reply, expected);
    }

    void shouldUnloadService_data()
    {
        QTest::addColumn<QString>("serviceName");

        QTest::newRow("printer service") << QString::fromUtf8("PrinterService");
        QTest::newRow("math service") << QString::fromUtf8("MathService");
    }

    void shouldUnloadService()
    {
        QFETCH(QString, serviceName);

        m_metaService.unloadService(serviceName);
    }
};

QTEST_MAIN(TestMetaService)

#include "testmetaservice.moc"
