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

        QString name;
        DataEngine::Data data;
        bool dirty;
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

QString DataSource::name()
{
    return objectName();
}

void DataSource::setName(const QString& name)
{
    setObjectName(name);
}

const DataEngine::Data DataSource::data() const
{
    return d->data;
}

void DataSource::setData(const QString& key, const QVariant& value)
{
    d->data[key] = value;
    d->dirty = true;
}

void DataSource::checkForUpdate()
{
    if (d->dirty) {
        emit updated(d->data);
        d->dirty = false;
    }
}

} // Plasma namespace

#include "datasource.moc"

