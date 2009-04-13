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

#include <QtCore/QEventLoop>
#include <QtCore/QObject>
#include <QtCore/QProcess>
#include <QtNetwork/QTcpSocket>
#include <QtTest/QtTest>

#include <sodepclient.h>
#include <sodepmessage.h>
#include "sodeptesthelpers.h"

#ifndef DATA_DIR
    #error "DATA_DIR is not set. A directory containing test jolie scripts is required for this test"
#endif


void dump(const SodepValue &value, int level)
{
    QString indent;

    while (level>0) {
        indent+="    ";
        level--;
    }

    qDebug() << (indent+value.toString()) << value.toInt() << value.toDouble();
    foreach (const QString &name, value.childrenNames()) {
        QList<SodepValue> children = value.children(name);
        qDebug() << (indent+"Children:") << name;
        foreach (const SodepValue &child, children) {
            dump(child, level+1);
        }
    }
}

void dump(const SodepMessage &message)
{
    qDebug() << "Resource :" << message.resourcePath();
    qDebug() << "Operation:" << message.operationName();
    qDebug() << "Fault    :" << message.fault().name();
    dump(message.fault().data(), 1);
    qDebug() << "Value    :";
    dump(message.data(), 1);
}

class SodepMetaServiceTest : public QObject
{
    Q_OBJECT

    QProcess m_metaserviceProcess;
    QTcpSocket m_socket;

public:
    SodepMetaServiceTest()
        : QObject()
    {
        qRegisterMetaType<SodepMessage>();
    }

private slots:
    void initTestCase()
    {
        m_metaserviceProcess.start("metaservice");
        QVERIFY2(m_metaserviceProcess.waitForStarted(), "Looks like you don't have Jolie's metaservice command");
        QTest::qWait(1000);

        m_socket.connectToHost("localhost", 9000);
        QVERIFY(m_socket.waitForConnected(-1));
    }

    void cleanupTestCase()
    {
        SodepMessage message("/", "shutdown");
        sodepWrite(m_socket, message);
        QTest::qWait(1000);

        m_socket.close();

        m_metaserviceProcess.waitForFinished();
    }

    void shouldLoadService_data()
    {
        QTest::addColumn<QString>("resourcePrefix");
        QTest::addColumn<QString>("fileName");

        QTest::newRow("printer service") << "Printer" << "printer.ol";
        QTest::newRow("math service") << "Math" << "math.ol";
    }

    void shouldLoadService()
    {
        QFETCH(QString, resourcePrefix);
        QFETCH(QString, fileName);

        SodepMessage message("/", "loadEmbeddedJolieService");
        SodepValue value;
        value.children("resourcePrefix") << SodepValue(resourcePrefix);
        value.children("filepath") << SodepValue(QString(DATA_DIR"/")+fileName);
        message.setData(value);
        sodepWrite(m_socket, message);

        SodepMessage reply = sodepReadMessage(m_socket);

        SodepMessage expected("/", "loadEmbeddedJolieService");
        expected.setData(SodepValue(resourcePrefix));

        sodepCompare(reply, expected);
    }

    void shouldListServices()
    {
        SodepClient client("localhost", 9000);

        SodepMessage message("/", "getServices");

        SodepMessage reply = client.call(message);
        SodepMessage expected("/", "getServices");
        SodepValue value;

        SodepValue s1;
        s1.children("isEmbedded") << SodepValue(1);
        s1.children("resourceName") << SodepValue("Math");
        SodepValue s2;
        s2.children("isEmbedded") << SodepValue(1);
        s2.children("resourceName") << SodepValue("Printer");

        value.children("service") << s1 << s2;
        expected.setData(value);

        sodepCompare(reply, expected);
    }

    void shouldPlaceServiceCalls_data()
    {
        QTest::addColumn<QString>("path");
        QTest::addColumn<QString>("method");
        QTest::addColumn<SodepValue>("data");
        QTest::addColumn<SodepValue>("replyData");

        QTest::newRow("printer service") << "/Printer" << "printInput" << SodepValue("Patapatapon!") << SodepValue("success");
        QTest::newRow("math service") << "/Math" << "twice" << SodepValue(10.5) << SodepValue(21.0);
    }

    void shouldPlaceServiceCalls()
    {
        QFETCH(QString, path);
        QFETCH(QString, method);
        QFETCH(SodepValue, data);
        QFETCH(SodepValue, replyData);

        SodepMessage message(path, method);
        message.setData(data);
        sodepWrite(m_socket, message);

        SodepMessage reply = sodepReadMessage(m_socket);

        SodepMessage expected("/", method);
        expected.setData(replyData);

        sodepCompare(reply, expected);
    }

    void shouldUnloadService_data()
    {
        QTest::addColumn<QString>("serviceName");

        QTest::newRow("printer service") << "PrinterService";
        QTest::newRow("math service") << "MathService";
    }

    void shouldUnloadService()
    {
        QFETCH(QString, serviceName);

        SodepMessage message("/", "unloadEmbeddedService");
        SodepValue value(serviceName);
        message.setData(value);
        sodepWrite(m_socket, message);

        SodepMessage reply = sodepReadMessage(m_socket);

        SodepMessage expected("/", "unloadEmbeddedService");

        sodepCompare(reply, expected);
    }
};

QTEST_MAIN(SodepMetaServiceTest)

#include "sodepmetaservicetest.moc"
