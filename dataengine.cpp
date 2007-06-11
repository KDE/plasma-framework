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

#include <QQueue>
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
            : engine(e),
              limit(0),
              valid(true)
        {
            updateTimer = new QTimer(engine);
            updateTimer->setSingleShot(true);
        }

        DataSource* source(const QString& sourceName, bool createWhenMissing = true)
        {
            DataEngine::SourceDict::const_iterator it = sources.find(sourceName);
            if (it != sources.constEnd()) {
                DataSource* s = it.value();
                if (limit > 0) {
                    QQueue<DataSource*>::iterator it = sourceQueue.begin();
                    while (it != sourceQueue.end()) {
                        if (*it == s) {
                            sourceQueue.erase(it);
                            break;
                        }
                        ++it;
                    }
                    sourceQueue.enqueue(s);
                }
                return it.value();
            }

            if (!createWhenMissing) {
                return 0;
            }

/*            kDebug() << "DataEngine " << engine->objectName()
                     << ": could not find DataSource " << sourceName
                     << ", creating" << endl;*/
            DataSource* s = new DataSource(engine);
            s->setObjectName(sourceName);
            sources.insert(sourceName, s);

            if (limit > 0) {
                trimQueue();
                sourceQueue.enqueue(s);
            }
            emit engine->newDataSource(sourceName);
            return s;
        }

        void trimQueue()
        {
            while (sourceQueue.count() >= limit) {
                DataSource* punted = sourceQueue.dequeue();
                engine->removeSource(punted->objectName());
            }
        }

        void queueUpdate()
        {
            if (updateTimer->isActive()) {
                return;
            }
            updateTimer->start(0);
        }

        bool dataSourceRequested(const QString& source)
        {
            //get around const! =P
            return engine->dataSourceRequested(source);
        }

        QAtomic ref;
        DataEngine::SourceDict sources;
        QQueue<DataSource*> sourceQueue;
        DataEngine* engine;
        QTimer* updateTimer;
        QString icon;
        uint limit;
        bool valid;
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
    //kDebug() << objectName() << ": bye bye birdy! " << endl;
    delete d;
}

QStringList DataEngine::dataSources() const
{
    return d->sources.keys();
}

void DataEngine::connectSource(const QString& source, QObject* visualization) const
{
    DataSource* s = d->source(source, false);

    if (!s) {
        // we didn't find a data source, so give the engine an opportunity to make one
        if (d->dataSourceRequested(source)) {
            s = d->source(source);
            if (s) {
                // now we have a source; since it was created on demand, assume
                // it should be removed when not used
                connect(s, SIGNAL(unused(QString)), this, SLOT(removeSource(QString)));
            }
        }
    }

    if (!s) {
        return;
    }

    connect(s, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
            visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
    QMetaObject::invokeMethod(visualization, SLOT(updated(QString,Plasma::DataEngine::Data)),
                              Q_ARG(QString, s->objectName()),
                              Q_ARG(Plasma::DataEngine::Data, s->data()));
}

void DataEngine::disconnectSource(const QString& source, QObject* visualization) const
{
    DataSource* s = d->source(source, false);

    if (!s) {
        return;
    }

    disconnect(s, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
               visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
}

void DataEngine::connectAllSources(QObject* visualization) const
{
    foreach (const DataSource* s, d->sources) {
        connect(s, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
                visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
    }

    foreach (const DataSource* s, d->sources) {
        QMetaObject::invokeMethod(visualization,
                                  SLOT(updated(QString,Plasma::DataEngine::Data)),
                                  Q_ARG(QString, s->objectName()),
                                  Q_ARG(Plasma::DataEngine::Data, s->data()));
    }
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

bool DataEngine::dataSourceRequested(const QString &name)
{
    Q_UNUSED(name)
    return false;
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
    SourceDict::const_iterator it = d->sources.find(source->objectName());
    if (it != d->sources.constEnd()) {
        kDebug() << "source named \"" << source->objectName() << "\" already exists." << endl;
        return;
    }

    d->sources.insert(source->objectName(), source);
    emit newDataSource(source->objectName());
}

void DataEngine::setSourceLimit(uint limit)
{
    if (d->limit == limit) {
        return;
    }

    d->limit = limit;

    if (d->limit > 0) {
        d->trimQueue();
    } else {
        d->sourceQueue.clear();
    }
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

void DataEngine::removeSource(const QString& source)
{
    //kDebug() << "removing source " << source << endl;
    SourceDict::iterator it = d->sources.find(source);
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

bool DataEngine::isValid() const
{
    return d->valid;
}

void DataEngine::setValid(bool valid)
{
    d->valid = valid;
}

DataEngine::SourceDict DataEngine::sourceDict() const
{
    return d->sources;
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
