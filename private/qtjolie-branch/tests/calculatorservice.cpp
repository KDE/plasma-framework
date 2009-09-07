/**
 * This file is part of the KDE project
 * Copyright (C) 2009 Kevin Ottens <ervin@kde.org>
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

#include <QtCore/QTimer>
#include <QtGui/QApplication>
#include <QtGui/QBoxLayout>
#include <QtGui/QTextEdit>
#include <QtNetwork/QTcpSocket>

#include <QtJolie/AbstractAdaptor>
#include <QtJolie/Server>
#include <QtJolie/Message>
#include <QtJolie/MetaService>

#include "testhelpers.h"

using namespace Jolie;

class CalculatorAdaptor : public AbstractAdaptor
{
    Q_OBJECT
public:
    CalculatorAdaptor(QObject *parent = 0)
        : AbstractAdaptor(parent)
    {
    }

signals:
    void added(int a, int b, int result);

private:
    void relay(Server *server, int clientId, const Message &message)
    {
        qint32 x = message.data().children("x").first().toInt();
        qint32 y = message.data().children("y").first().toInt();

        emit added(x, y, x+y);
        sodepDump(message);

        Message reply(message.resourcePath(),
                      message.operationName(),
                      message.id());
        reply.setData(Value(x+y));

        server->sendReply(clientId, reply);
    }
};

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow()
    {
        setLayout(new QHBoxLayout());
        m_textEdit = new QTextEdit(this);
        layout()->addWidget(m_textEdit);

        m_meta.start();
        QTimer::singleShot(1000, this, SLOT(delayedInit()));
    }

    ~MainWindow()
    {
        m_meta.stop();
        delete m_server;
    }

private slots:
    void delayedInit()
    {
        m_server = new Server(8000);
        CalculatorAdaptor *adaptor = new CalculatorAdaptor(this);
        connect(adaptor, SIGNAL(added(int, int, int)),
                this, SLOT(onAdded(int, int, int)));
        m_server->registerAdaptor("/", adaptor);

        m_meta.addRedirection(QString::fromUtf8("Calculator"),
                              QString::fromUtf8("socket://localhost:8000/"),
                              Value("sodep"),
                              Value("soap"));
    }

    void onAdded(int a, int b, int result)
    {
        m_textEdit->append(QString::number(a)
                         + QString::fromUtf8(" + ")
                         + QString::number(b)
                         + QString::fromUtf8(" = ")
                         + QString::number(result));
    }

private:
    MetaService m_meta;
    QTextEdit *m_textEdit;
    Server *m_server;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}

#include "calculatorservice.moc"
