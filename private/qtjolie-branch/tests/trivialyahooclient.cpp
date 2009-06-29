/**
 * This file is part of the KDE project
 * Copyright (C) 2009 Kevin Ottens <ervin@kde.org>
 * Copyright (C) 2009 Fabrizio Montesi <famontesi@gmail.com>
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

#include <QtJolie/Client>
#include <QtJolie/Message>
#include <QtJolie/MetaService>

using namespace Jolie;

class MainWindow : public QWidget
{
    Q_OBJECT
public:
    MainWindow()
    {
        setEnabled(false);

        setLayout(new QVBoxLayout());
        m_lineEdit = new QLineEdit(this);
        m_browser = new QTextBrowser(this);
        m_browser->setOpenExternalLinks(true);
        layout()->addWidget(m_lineEdit);
        layout()->addWidget(m_browser);

        m_meta.start();
        QTimer::singleShot(1000, this, SLOT(delayedInit()));
    }

    ~MainWindow()
    {
        m_meta.stop();
        delete m_client;
    }

private slots:
    void delayedInit()
    {
        Value aliases;
        aliases.children("webSearch") << Value("webSearch?appid=%{applicationId}&query=%{query}");
        Value inProtocol("http");
        inProtocol.children("format") << Value("rest");
        inProtocol.children("aliases") << aliases;
        Value outProtocol("sodep");

        m_resourcePath = m_meta.addRedirection(
            QString::fromUtf8("YahooWebSearch"),
            QString::fromUtf8("socket://search.yahooapis.com:80/WebSearchService/V1/"),
            inProtocol,
            outProtocol
        );

        m_client = new Client(QString::fromUtf8("localhost"), 9000);
        connect(m_lineEdit, SIGNAL(returnPressed()),
                this, SLOT(onReturnPressed()));

        setEnabled(true);
        m_lineEdit->setFocus();
    }

    void onReturnPressed()
    {
        Message request((QString::fromUtf8("/")+m_resourcePath).toUtf8(), "webSearch");
        Value requestData;
        requestData.children("applicationId") << Value("UrlfFCDV34F.d8xYpsswjaA7P.unhHJ6ELvrhhIb.ybJeUHh.POHIkSf_YO0Fw--");
        requestData.children("query") << Value(m_lineEdit->text().toUtf8());
        request.setData(requestData);

        Message response = m_client->call(request);

        m_browser->clear();
        QString html = QString::fromUtf8("<html><body>");

        foreach (const Value &value, response.data().children("Result")) {
            QString url = QString::fromUtf8(value.children("Url").first().toByteArray());
            QString title = QString::fromUtf8(value.children("Title").first().toByteArray());
            QString summary = QString::fromUtf8(value.children("Summary").first().toByteArray());

            html+= QString::fromUtf8(
                            "<div><a href=\"%1\"><b>%2</b></a></div>"
                            "<div>%3</div><br/>"
                            ).arg(url).arg(title).arg(summary);
        }

        html+= QString::fromUtf8("</body></html>");

        m_browser->setHtml(html);
    }

private:
    MetaService m_meta;
    QLineEdit *m_lineEdit;
    QTextBrowser *m_browser;
    Client *m_client;
    QString m_resourcePath;
};

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MainWindow w;
    w.show();

    return app.exec();
}

#include "trivialyahooclient.moc"
