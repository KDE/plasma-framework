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

#include <qtjolie/client.h>
#include <qtjolie/message.h>
#include <qtjolie/pendingcall.h>
#include "testhelpers.h"

#ifndef DATA_DIR
    #error "DATA_DIR is not set. A directory containing test jolie scripts is required for this test"
#endif

using namespace Jolie;

void dump(const Value &value, int level)
{
    QString indent;

    while (level>0) {
        indent+="    ";
        level--;
    }

    qDebug() << (indent+value.toString()) << value.toInt() << value.toDouble();
    foreach (const QString &name, value.childrenNames()) {
        QList<Value> children = value.children(name);
        qDebug() << (indent+"Children:") << name;
        foreach (const Value &child, children) {
            dump(child, level+1);
        }
    }
}

void dump(const Message &message)
{
    qDebug() << "Resource :" << message.resourcePath();
    qDebug() << "Operation:" << message.operationName();
    qDebug() << "Fault    :" << message.fault().name();
    dump(message.fault().data(), 1);
    qDebug() << "Value    :";
    dump(message.data(), 1);
}

class TestMetaService : public QObject
{
    Q_OBJECT

    QProcess m_metaserviceProcess;
    Client *m_client;

public:
    TestMetaService()
        : QObject()
    {
        qRegisterMetaType<Message>();
    }

private slots:
    void initTestCase()
    {
        m_metaserviceProcess.start("metaservice");
        QVERIFY2(m_metaserviceProcess.waitForStarted(), "Looks like you don't have Jolie's metaservice command");
        QTest::qWait(1000);

        m_client = new Client("localhost", 9000);
    }

    void cleanupTestCase()
    {
        Message message("/", "shutdown");
        m_client->callNoReply(message);
        delete m_client;

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

        Message message("/", "loadEmbeddedJolieService");
        Value value;
        value.children("resourcePrefix") << Value(resourcePrefix);
        value.children("filepath") << Value(QString(DATA_DIR"/")+fileName);
        message.setData(value);

        Message reply = m_client->call(message);

        Message expected("/", "loadEmbeddedJolieService");
        expected.setData(Value(resourcePrefix));

        sodepCompare(reply, expected);
    }

    void shouldListServices()
    {
        Message message("/", "getServices");

        Message reply = m_client->call(message);
        Message expected("/", "getServices");
        Value value;

        Value s1;
        s1.children("isEmbedded") << Value(1);
        s1.children("resourceName") << Value("Math");
        Value s2;
        s2.children("isEmbedded") << Value(1);
        s2.children("resourceName") << Value("Printer");

        value.children("service") << s1 << s2;
        expected.setData(value);

        sodepCompare(reply, expected);
    }

    void shouldPlaceServiceCalls_data()
    {
        QTest::addColumn<QString>("path");
        QTest::addColumn<QString>("method");
        QTest::addColumn<Value>("data");
        QTest::addColumn<Value>("replyData");

        QTest::newRow("printer service") << "/Printer" << "printInput" << Value("Patapatapon!") << Value("success");
        QTest::newRow("math service") << "/Math" << "twice" << Value(10.5) << Value(21.0);
    }

    void shouldPlaceServiceCalls()
    {
        QFETCH(QString, path);
        QFETCH(QString, method);
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

        QTest::newRow("printer service") << "PrinterService";
        QTest::newRow("math service") << "MathService";
    }

    void shouldUnloadService()
    {
        QFETCH(QString, serviceName);

        Message message("/", "unloadEmbeddedService");
        Value value(serviceName);
        message.setData(value);

        Message reply = m_client->call(message);

        Message expected("/", "unloadEmbeddedService");

        sodepCompare(reply, expected);
    }
};

QTEST_MAIN(TestMetaService)

#include "testmetaservice.moc"
