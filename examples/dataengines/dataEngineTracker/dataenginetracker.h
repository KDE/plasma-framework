/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef DATAENGINETRACKER_H
#define DATAENGINETRACKER_H

#include <QObject>

#include <Plasma/DataEngine>

class DataEngineTracker : QObject
{
    Q_OBJECT

public:
    DataEngineTracker(Plasma::DataEngine *engine, QObject *parent = nullptr);

public Q_SLOTS:
    void dataUpdated(const QString &source, const Plasma::DataEngine::Data &data);
    void sourceAdded(const QString &source);
    void sourceRemoved(const QString &source);

private:
    Plasma::DataEngine *m_engine;
};

#endif
