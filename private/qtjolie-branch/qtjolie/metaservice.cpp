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

#include "metaservice.h"

#include <QtCore/QProcess>

#include "client.h"
#include "message.h"

namespace Jolie
{

class MetaServicePrivate
{
public:
    QProcess metaserviceProcess;
};

} // namespace Jolie

using namespace Jolie;

MetaService::MetaService()
    : d(new MetaServicePrivate)
{
}

MetaService::~MetaService()
{
    delete d;
}

bool MetaService::start()
{
    d->metaserviceProcess.start(QString::fromUtf8("metaservice"));
    return d->metaserviceProcess.waitForStarted();
}

bool MetaService::stop()
{
    Client client(QString::fromUtf8("localhost"), 9000);
    Message message("/", "shutdown");
    client.callNoReply(message);
    return d->metaserviceProcess.waitForFinished(30000);
}

bool MetaService::isRunning() const
{
    return d->metaserviceProcess.state()==QProcess::Running;
}

QString MetaService::loadService(const QString &name, const QString &fileName)
{
    Client client(QString::fromUtf8("localhost"), 9000);
    Message message("/", "loadEmbeddedJolieService");
    Value value;
    value.children("resourcePrefix") << Value(name.toUtf8());
    value.children("filepath") << Value(fileName.toUtf8());
    message.setData(value);

    Message reply = client.call(message);
    return QString::fromUtf8(reply.data().toByteArray());
}

QStringList MetaService::loadedServices() const
{
    Client client(QString::fromUtf8("localhost"), 9000);
    Message message("/", "getServices");

    Message reply = client.call(message);
    QList<Value> services = reply.data().children("service");

    QStringList result;
    foreach (const Value &service, services) {
        result << QString::fromUtf8(service.children("resourceName").first().toByteArray());
    }

    return result;
}

void MetaService::unloadService(const QString &name)
{
    Client client(QString::fromUtf8("localhost"), 9000);
    Message message("/", "unloadEmbeddedService");
    message.setData(Value(name.toUtf8()));

    client.call(message);
}

QString Jolie::MetaService::addRedirection(const QString &name, const QString &url, const Value &inProtocol, const Value &outProtocol)
{
    Client client(QString::fromUtf8("localhost"), 9000);
    Message message("/", "addRedirection");
    Value value;
    value.children("resourcePrefix") << Value(name.toUtf8());
    value.children("location") << Value(url.toUtf8());
    value.children("protocol") << inProtocol;
    value.children("exposedProtocol") << outProtocol;
    message.setData(value);

    Message reply = client.call(message);
    return QString::fromUtf8(reply.data().toByteArray());
}
