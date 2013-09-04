/*
 *   Copyright 2011 Aaron Seigo <aseigo@kde.org>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 *    THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *    IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 *    OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 *    IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 *    INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 *    NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 *   THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "dataenginetracker.h"

#include <QTime>

DataEngineTracker::DataEngineTracker(Plasma::DataEngine *engine, QObject *parent)
    : QObject(parent),
      m_engine(engine)
{
    connect(engine, SIGNAL(sourceAdded(QString)), this, SLOT(sourceAdded(QString)));
    connect(engine, SIGNAL(sourceRemoved(QString)), this, SLOT(sourceRemoved(QString)));
    engine->connectAllSources(this);
}

void DataEngineTracker::dataUpdated(const QString &source, const Plasma::DataEngine::Data &data)
{
    kDebug() << QTime::currentTime() << source;
    QHashIterator<QString, QVariant> it(data);
    while (it.hasNext()) {
        it.next();
        kDebug() << "     " << it.key() << it.value();
    }
}

void DataEngineTracker::sourceAdded(const QString &source)
{
    kDebug() << QTime::currentTime() << source;
    m_engine->connectSource(source, this);
}

void DataEngineTracker::sourceRemoved(const QString &source)
{
    kDebug() << QTime::currentTime() << source;
}

#include <dataenginetracker.moc>

