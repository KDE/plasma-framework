/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLUGINLOADERTEST_H
#define PLUGINLOADERTEST_H

#include <QCoreApplication>
#include <plasma/dataengine.h>

class PluginTest : public QObject
{
    Q_OBJECT
public:
    PluginTest();

private Q_SLOTS:
    void listEngines();
    void listAppletCategories();
    void listContainmentActions();
    void listContainmentsOfType();

    void loadDataEngine();

private:
    bool m_buildonly;
};

class EngineTest : public QObject
{
    Q_OBJECT
public:
    EngineTest()
    {
    }

public Q_SLOTS:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
};

#endif
