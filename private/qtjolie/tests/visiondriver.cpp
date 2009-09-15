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

#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include <QtGui/QApplication>
#include <QtGui/QBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QTextBrowser>

#include <QtJolie/AbstractAdaptor>
#include <QtJolie/Client>
#include <QtJolie/Message>
#include <QtJolie/Server>

#include <unistd.h>

#include "ui_visiondriverwidget.h"

using namespace Jolie;

class MainWindow;

class PresenterAdaptor : public AbstractAdaptor
{
    Q_OBJECT
public:
    PresenterAdaptor(MainWindow *parent);

private:
    void relay(Server *server, int clientId, const Message &message);

    MainWindow *m_window;
};

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow()
        : m_client(0), m_server(0), m_sessionId(-1)
    {
        ui.setupUi(this);

        m_server = new Server(9002);
        PresenterAdaptor *adaptor = new PresenterAdaptor(this);
        m_server->registerAdaptor("/", adaptor);
        sleep(2);

        connect(ui.connectButton, SIGNAL(clicked()), SLOT(onConnect()));
        connect(ui.previousButton, SIGNAL(clicked()), SLOT(onPrevious()));
        connect(ui.nextButton, SIGNAL(clicked()), SLOT(onNext()));
    }

    ~MainWindow()
    {
        if (m_sessionId>=0) {
            Message m("/", "closeClientSession");
            m.setData(Value(m_sessionId));
            m_client->callNoReply(m);
            sleep(2);
        }

        delete m_client;
        delete m_server;
    }

    void pageChanged(int pageNumber)
    {
        qDebug() << "New page is:" << pageNumber;
        m_currentPage = pageNumber;
        updateTextContent();
    }

    void documentChanged(const QString &url)
    {
        qDebug() << "New document is:" << url;
        m_currentDocument = url;
        updateTextContent();
    }

private slots:
    void onConnect()
    {
        if (m_sessionId>=0) {
            Message m("/", "closeClientSession");
            m.setData(Value(m_sessionId));
            m_client->callNoReply(m);
            sleep(2);
        }

        m_sessionId = -1;
        delete m_client;

        m_client = new Client(ui.visionHostname->text(),
                              ui.visionPort->value());

        QString location = QString::fromUtf8("socket://%1:%2")
                           .arg(ui.driverHostname->text())
                           .arg(ui.driverPort->value());
        Value data;
        data.children("location") << Value(location.toUtf8());

        Message request("/", "startClientSession");
        request.setData(data);
        Message reply = m_client->call(request);

        data = reply.data();
        m_sessionId = data.children("sid").first().toInt();
        qDebug() << "Got session ID:" << m_sessionId;
    }

    void onPrevious()
    {
        requestNewPage(m_currentPage-1);
    }

    void onNext()
    {
        requestNewPage(m_currentPage+1);
    }

private:
    void updateTextContent()
    {
        ui.textContent->setHtml(
            QString::fromUtf8(
                "<div>%1</div><br/><br/>"
                "<div align=\"center\"><h1>%2</h1></div><br/><br/>"
                ).arg(m_currentDocument).arg(m_currentPage)
            );
    }

    void requestNewPage(int page)
    {
        Value data;
        data.children("pageNumber") << Value(page);
        data.children("local") << Value(0);

        Message request("/", "goToPage");
        request.setData(data);
        m_client->call(request);
    }

    Ui::VisionDriverWidget ui;
    Client *m_client;
    Server *m_server;
    int m_sessionId;

    int m_currentPage;
    QString m_currentDocument;
};

PresenterAdaptor::PresenterAdaptor(MainWindow *parent)
    : AbstractAdaptor(parent), m_window(parent)
{
}

void PresenterAdaptor::relay(Server *server, int clientId, const Message &message)
{
    if ((message.resourcePath()=="/") && (message.operationName()=="goToPage")) {
        int pageNumber = 0;
        if (message.data().isValid()) {
            pageNumber = message.data().toInt();
        } else {
            pageNumber = message.data().children("pageNumber").first().toInt();
        }

        m_window->pageChanged(pageNumber);
    } else if ((message.resourcePath()=="/") && (message.operationName()=="openDocument")) {
        QString url;
        if (message.data().isValid()) {
            url = QString::fromUtf8(message.data().toByteArray());
        } else {
            url = QString::fromUtf8(message.data().children("documentUrl").first().toByteArray());
        }
        m_window->documentChanged(url);
    } else {
        qDebug() << "Got an unknown message:" << message.resourcePath()
                 << message.operationName();
    }

    server->sendReply(clientId, Message());
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}

#include "visiondriver.moc"
