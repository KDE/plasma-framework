/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.
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

#include "datacontainer.h"

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

        DataContainer* source(const QString& sourceName, bool createWhenMissing = true)
        {
            DataEngine::SourceDict::const_iterator it = sources.find(sourceName);
            if (it != sources.constEnd()) {
                DataContainer* s = it.value();
                if (limit > 0) {
                    QQueue<DataContainer*>::iterator it = sourceQueue.begin();
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
                     << ": could not find DataContainer " << sourceName
                     << ", creating" << endl;*/
            DataContainer* s = new DataContainer(engine);
            s->setObjectName(sourceName);
            sources.insert(sourceName, s);

            if (limit > 0) {
                trimQueue();
                sourceQueue.enqueue(s);
            }
            emit engine->newSource(sourceName);
            return s;
        }

        DataContainer* requestSource(const QString& sourceName)
        {
            DataContainer* s = source(sourceName, false);

            if (!s) {
                // we didn't find a data source, so give the engine an opportunity to make one
                if (engine->sourceRequested(sourceName)) {
                    s = source(sourceName, false);
                    if (s) {
                        // now we have a source; since it was created on demand, assume
                        // it should be removed when not used
                        connect(s, SIGNAL(unused(QString)), engine, SLOT(removeSource(QString)));
                    }
                }
            }
            return s;
        }

        void trimQueue()
        {
            while (sourceQueue.count() >= limit) {
                DataContainer* punted = sourceQueue.dequeue();
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

        int ref;
        DataEngine::SourceDict sources;
        QQueue<DataContainer*> sourceQueue;
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
    //Update DataEngine::init() api docu when fixed
    QTimer::singleShot(0, this, SLOT(startInit()));
}

DataEngine::~DataEngine()
{
    //kDebug() << objectName() << ": bye bye birdy! ";
    delete d;
}

QStringList DataEngine::sources() const
{
    return d->sources.keys();
}

void DataEngine::connectSource(const QString& source, QObject* visualization) const
{
    DataContainer* s = d->requestSource(source);

    if (!s) {
        return;
    }

    connect(visualization, SIGNAL(destroyed(QObject*)), s, SLOT(checkUsage()), Qt::QueuedConnection);
    connect(s, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
            visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
    QMetaObject::invokeMethod(visualization, "updated",
                              Q_ARG(QString, s->objectName()),
                              Q_ARG(Plasma::DataEngine::Data, s->data()));
}

void DataEngine::disconnectSource(const QString& source, QObject* visualization) const
{
    DataContainer* s = d->source(source, false);

    if (!s) {
        return;
    }

    disconnect(s, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
               visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
}

void DataEngine::connectAllSources(QObject* visualization) const
{
    foreach (const DataContainer* s, d->sources) {
        connect(s, SIGNAL(updated(QString,Plasma::DataEngine::Data)),
                visualization, SLOT(updated(QString,Plasma::DataEngine::Data)));
    }

    foreach (const DataContainer* s, d->sources) {
        QMetaObject::invokeMethod(visualization, "updated",
                                  Q_ARG(QString, s->objectName()),
                                  Q_ARG(Plasma::DataEngine::Data, s->data()));
    }
}

DataContainer* DataEngine::containerForSource(const QString &source)
{
    return d->requestSource(source);
}

DataEngine::Data DataEngine::query(const QString& source) const
{
    DataContainer* s = d->requestSource(source);

    if (!s) {
        return DataEngine::Data();
    }

    DataEngine::Data data = s->data();
    s->checkUsage();
    return data;
}

void DataEngine::startInit()
{
    init();
}

void DataEngine::init()
{
    // kDebug() << "DataEngine::init() called ";
    // default implementation does nothing. this is for engines that have to
    // start things in motion external to themselves before they can work
}

bool DataEngine::sourceRequested(const QString &name)
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
    DataContainer* s = d->source(source);
    s->setData(key, value);
    d->queueUpdate();
}

void DataEngine::setData(const QString &source, const Data &data)
{
    DataContainer *s = d->source(source);
    Data::const_iterator it = data.constBegin();
    while (it != data.constEnd()) {
        s->setData(it.key(), it.value());
        ++it;
    }
    d->queueUpdate();
}


void DataEngine::clearData(const QString& source)
{
    DataContainer* s = d->source(source, false);
    if (s) {
        s->clearData();
        d->queueUpdate();
    }
}

void DataEngine::removeData(const QString& source, const QString& key)
{
    DataContainer* s = d->source(source, false);
    if (s) {
        s->setData(key, QVariant());
        d->queueUpdate();
    }
}

void DataEngine::addSource(DataContainer* source)
{
    SourceDict::const_iterator it = d->sources.find(source->objectName());
    if (it != d->sources.constEnd()) {
        kDebug() << "source named \"" << source->objectName() << "\" already exists.";
        return;
    }

    d->sources.insert(source->objectName(), source);
    emit newSource(source->objectName());
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

void DataEngine::removeSource(const QString& source)
{
    //kDebug() << "removing source " << source;
    SourceDict::iterator it = d->sources.find(source);
    if (it != d->sources.end()) {
        emit sourceRemoved(it.key());
        it.value()->deleteLater();
        d->sources.erase(it);
    }
}

void DataEngine::clearSources()
{
    QMutableHashIterator<QString, Plasma::DataContainer*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        emit sourceRemoved(it.key());
        delete it.value();
        it.remove();
    }
}

void DataEngine::ref()
{
    --d->ref;
}

void DataEngine::deref()
{
    ++d->ref;
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
    QHashIterator<QString, Plasma::DataContainer*> it(d->sources);
    while (it.hasNext()) {
        it.next();
        it.value()->checkForUpdate();
    }
}

}

#include "dataengine.moc"
