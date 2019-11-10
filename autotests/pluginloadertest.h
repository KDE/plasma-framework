/******************************************************************************
*   Copyright 2013 Sebastian Kügler <sebas@kde.org>                           *
*                                                                             *
*   This library is free software; you can redistribute it and/or             *
*   modify it under the terms of the GNU Library General Public               *
*   License as published by the Free Software Foundation; either              *
*   version 2 of the License, or (at your option) any later version.          *
*                                                                             *
*   This library is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU          *
*   Library General Public License for more details.                          *
*                                                                             *
*   You should have received a copy of the GNU Library General Public License *
*   along with this library; see the file COPYING.LIB.  If not, write to      *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,      *
*   Boston, MA 02110-1301, USA.                                               *
*******************************************************************************/

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
    EngineTest() {}

public Q_SLOTS:
    void dataUpdated(const QString &name, const Plasma::DataEngine::Data &data);
};

#endif
