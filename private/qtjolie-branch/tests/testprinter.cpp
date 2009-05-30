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

#include <QtCore/QBuffer>
#include <QtGui/QApplication>
#include <QtGui/QBoxLayout>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtNetwork/QTcpSocket>

#include <QtJolie/Message>
#include <QtJolie/Value>
#include "../qtjolie/sodephelpers_p.h"

using namespace Jolie;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow()
    {
        setLayout(new QHBoxLayout());
        m_lineEdit = new QLineEdit(this);
        layout()->addWidget(m_lineEdit);

        QPushButton *button = new QPushButton(this);
        button->setText(QString::fromUtf8("SEND"));
        layout()->addWidget(button);
        connect(button, SIGNAL(clicked()),
                this, SLOT(sendMessage()));

        m_socket.connectToHost(QString::fromUtf8("localhost"), 10000);
        if (!m_socket.waitForConnected(10000)) {
            qDebug("Failed to connect!");
            return;
        }
        qDebug("Connected!");
    }

private slots:
    void sendMessage()
    {
        Message message("/", "printInput");
        message.setData(Value(m_lineEdit->text().toUtf8()));
        sodepWrite(m_socket, message);

        qDebug("Message sent:");
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        sodepWrite(buffer, message);
        buffer.close();
        qDebug("%s", buffer.data().toHex().constData());

        qDebug("Message received:");
        buffer.setData(QByteArray());
        buffer.open(QIODevice::WriteOnly);
        message = sodepReadMessage(m_socket);
        sodepWrite(buffer, message);
        buffer.close();
        qDebug("%s", buffer.data().toHex().constData());
    }
private:
    QLineEdit *m_lineEdit;
    QTcpSocket m_socket;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}

#include "testprinter.moc"
