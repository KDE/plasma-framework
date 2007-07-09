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

#include "datasource.h"

#include <QAtomic>
#include <QVariant>

#include <KDebug>

namespace Plasma
{

class DataSource::Private
{
    public:
        Private()
            : dirty(false)
        {}

        DataEngine::Data data;
        QAtomic connectCount;
        bool dirty : 1;
};

DataSource::DataSource(QObject* parent)
    : QObject(parent),
      d(new Private())
{
}

DataSource::~DataSource()
{
    delete d;
}

const DataEngine::Data DataSource::data() const
{
    return d->data;
}

void DataSource::setData(const QString& key, const QVariant& value)
{
    if (value.isNull() || !value.isValid()) {
        if (!d->data.contains(key)) {
            return;
        }

        d->data.remove(key);
    } else {
        d->data[key] = value;
    }

    d->dirty = true;
}

void DataSource::clearData()
{
    if (d->data.count() < 1) {
        // avoid an update if we don't have any data anyways
        return;
    }

    d->data.clear();
    d->dirty = true;
}

void DataSource::checkForUpdate()
{
    if (d->dirty) {
        emit updated(objectName(), d->data);
        d->dirty = false;
    }
}

void DataSource::connectNotify(const char *signal)
{
    if (QLatin1String(signal) == QMetaObject::normalizedSignature(SIGNAL(updated(QString, Plasma::DataEngine::Data))).constData()) {
        d->connectCount.ref();
    }
}

void DataSource::disconnectNotify(const char *signal)
{
    if (QLatin1String(signal) == QMetaObject::normalizedSignature(SIGNAL(updated(QString, Plasma::DataEngine::Data))).constData()) {
        if (d->connectCount > 0) {
            d->connectCount.deref();
        }

        if (d->connectCount < 1) {
            // DO NOT CALL ANYTHING AFTER THIS LINE AS IT MAY GET DELETED!
            emit unused(objectName());
        }
    }
}

} // Plasma namespace

#include "datasource.moc"

