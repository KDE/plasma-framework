/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <kdebug.h>
#include <QVariant>

#include "dataengine.h"
#include "dataengine.moc"

using namespace Plasma;

class DataSource::Private
{
    public:
        Private() {}
        ~Private() {}
};

DataSource::DataSource(QObject* parent)
    : QObject(parent)
{
    d = new Private();
}

DataSource::~DataSource()
{
    delete d;
}

QString DataSource::name()
{
    kDebug() << k_funcinfo << " not implemented";
    return QString();
}






DataEngine::DataEngine(QObject* parent)
    : QObject(parent)
{
}

DataEngine::~DataEngine()
{
}

QStringList DataEngine::dataSources()
{
    kDebug() << k_funcinfo << " not implemented";
    return QStringList();
}

void DataEngine::connect(const QString& source, DataVisualization* visualization)
{
    Q_UNUSED(source)
    Q_UNUSED(visualization)

    kDebug() << k_funcinfo << " not implemented";
}

DataSource::Data DataEngine::query(const QString& source)
{
    Q_UNUSED(source)

    kDebug() << k_funcinfo << " not implemented";
    return DataSource::Data();
}

void DataEngine::init()
{
    kDebug() << k_funcinfo << " not implemented";
}

void DataEngine::cleanup()
{
    kDebug() << k_funcinfo << " not implemented";
}

void DataEngine::setDataSource(const QString& source, const QVariant& value)
{
    Q_UNUSED(source)
    Q_UNUSED(value)

    kDebug() << k_funcinfo << " not implemented";
}

void DataEngine::createDataSource(const QString& source, const QString& domain)
{
    Q_UNUSED(source)
    Q_UNUSED(domain)

    kDebug() << k_funcinfo << " not implemented";
}

void DataEngine::removeDataSource(const QString& source)
{
    Q_UNUSED(source)

    kDebug() << k_funcinfo << " not implemented";
}

void DataEngine::clearAllDataSources()
{
    kDebug() << k_funcinfo << " not implemented";
}

void DataEngine::ref()
{
    kDebug() << k_funcinfo << " not implemented";
}

void DataEngine::deref()
{
    kDebug() << k_funcinfo << " not implemented";
}

bool DataEngine::isUsed()
{
    kDebug() << k_funcinfo << " not implemented";
    return false;
}


