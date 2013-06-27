/******************************************************************************
*   Copyright 2012 Sebastian Kügler <sebas@kde.org>                           *
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

#ifndef PLUGINTEST_H
#define PLUGINTEST_H

#include <QCoreApplication>
#include <QApplication>

#include <Plasma/DataEngine>

class QCommandLineParser;
class KJob;

namespace Plasma
{

class PluginTestPrivate;

class PluginTest : public QApplication
{
    Q_OBJECT

    public:
        PluginTest(int& argc, char** argv, QCommandLineParser *parser);
        virtual ~PluginTest();

        void showPackageInfo(const QString &pluginName);

    public Q_SLOTS:
        void runMain();
        void loadKQPlugin();
        void loadDataEngine(const QString &name = QString());
        void dataUpdated(QString s, Plasma::DataEngine::Data d);

    private:
        PluginTestPrivate* d;
};

}

#endif
