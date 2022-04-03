/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLUGINTEST_H
#define PLUGINTEST_H

#include <QApplication>

#include <Plasma/DataEngine>

class QCommandLineParser;

namespace Plasma
{
class PluginTestPrivate;

class PluginTest : public QApplication
{
    Q_OBJECT

public:
    PluginTest(int &argc, char **argv, QCommandLineParser *parser);
    ~PluginTest() override;

public Q_SLOTS:
    void runMain();
    bool loadKPlugin();
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 94)
    bool loadFromPlasma();
#endif
    void loadKQPlugin();
    void dataUpdated(QString s, Plasma::DataEngine::Data d);

private:
    PluginTestPrivate *d;
};

}

#endif
