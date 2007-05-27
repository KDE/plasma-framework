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

#include "dataengine.h"

#include <QTimer>
#include <QVariant>

#include <KDebug>

#include "datasource.h"

namespace Plasma
{

class DataEngine::Private
{
    public:
        Private(DataEngine* e)
            : engine(e)
        {
            updateTimer = new QTimer(engine);
            updateTimer->setSingleShot(true);
        }

        DataSource* source(const QString& sourceName)
        {
            DataSource::Dict::const_iterator it = sources.find(sourceName);
            if (it != sources.constEnd()) {
                return it.value();
            }

            kDebug() << "DataEngine " << engine->objectName()
                     << ": could not find DataSource " << sourceName
                     << ", creating" << endl;
            DataSource* s = new DataSource(engine);
            s->setObjectName(sourceName);
            sources.insert(sourceName, s);
            emit engine->newDataSource(sourceName);
            return s;
        }

        void queueUpdate()
        {
            if (updateTimer->isActive()) {
                return;
            }
            updateTimer->start(0);
        }

        QAtomic ref;
        DataSource::Dict sources;
        DataEngine* engine;
        QTimer* updateTimer;
        QString icon;
};


DataEngine::DataEngine(QObject* parent)
    : QObject(parent),
      d(new Private(this))
{
    connect(d->updateTimer, SIGNAL(timeout()), this, SLOT(checkForUpdates()));
    //FIXME: we should delay this call; to when is the question.
    init();
}

DataEngine::~DataEngine()
{
    delete d;
}

QStringList DataEngine::dataSources() const
{
    return d->sources.keys();
}

void DataEngine::connectSource(const QString& source, QObject* visualization) const
{
    Q_UNUSED(source)
    Q_UNUSED(visualization)

    DataSource* s = d->source(source);
    connect(s, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
            visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
}

DataEngine::Data DataEngine::query(const QString& source) const
{
    Q_UNUSED(source)

    DataSource* s = d->source(source);
    return s->data();
}

void DataEngine::init()
{
    // default implementation does nothing. this is for engines that have to
    // start things in motion external to themselves before they can work
}

void DataEngine::setData(const QString& source, const QVariant& value)
{
    setData(source, source, value);
}

void DataEngine::setData(const QString& source, const QString& key, const QVariant& value)
{
    DataSource* s = d->source(source);
    s->setData(key, value);
    d->queueUpdate();
}

void DataEngine::addSource(DataSource* source)
{
    DataSource::Dict::const_iterator it = d->sources.find(source->objectName());
    if (it != d->sources.constEnd()) {
        kDebug() << "source named \"" << source->objectName() << "\" already exists." << endl;
        return;
    }

    d->sources.insert(source->objectName(), source);
    emit newDataSource(source->objectName());
}

/*
Plasma::DataSource* DataEngine::createDataSource(const QString& source, const QString& domain)
{
    Q_UNUSED(domain)
    //TODO: add support for domains of sources
    
    if (d->source(source)) {
        kDebug() << "DataEngine " << objectName() << ": source "  << source << " already exists " << endl;
        return s
    }
}*/

void DataEngine::removeDataSource(const QString& source)
{
    DataSource::Dict::iterator it = d->sources.find(source);
    if (it != d->sources.end()) {
        emit dataSourceRemoved(it.key());
        d->sources.erase(it);
    }
}

void DataEngine::clearAllDataSources()
{
    QMutableHashIterator<QString, Plasma::DataSource*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        emit dataSourceRemoved(it.key());
        delete it.value();
        it.remove();
    }
}

void DataEngine::ref()
{
    d->ref.ref();
}

void DataEngine::deref()
{
    d->ref.deref();
}

bool DataEngine::isUsed() const
{
    return d->ref != 0;
}

void DataEngine::setIcon(const QString& icon)
{
    d->icon = icon;
}

QString DataEngine::icon() const
{
    return d->icon;
}

void DataEngine::checkForUpdates()
{
    QHashIterator<QString, Plasma::DataSource*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        it.value()->checkForUpdate();
    }
}

}

#include "dataengine.moc"

