/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#include "dataenginetracker.h"

#include <QDebug>
#include <QTime>

DataEngineTracker::DataEngineTracker(Plasma::DataEngine *engine, QObject *parent)
    : QObject(parent)
    , m_engine(engine)
{
    connect(engine, SIGNAL(sourceAdded(QString)), this, SLOT(sourceAdded(QString)));
    connect(engine, SIGNAL(sourceRemoved(QString)), this, SLOT(sourceRemoved(QString)));
    engine->connectAllSources(this);
}

void DataEngineTracker::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    qDebug() << QTime::currentTime() << source;
    QHashIterator<QString, QVariant> it(data);
    while (it.hasNext()) {
        it.next();
        qDebug() << "     " << it.key() << it.value();
    }
}

void DataEngineTracker::sourceAdded(const QString &source)
{
    qDebug() << QTime::currentTime() << source;
    m_engine->connectSource(source, this);
}

void DataEngineTracker::sourceRemoved(const QString &source)
{
    qDebug() << QTime::currentTime() << source;
}

#include <dataenginetracker.moc>
