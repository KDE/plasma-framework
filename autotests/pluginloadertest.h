/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef PLUGINLOADERTEST_H
#define PLUGINLOADERTEST_H

#include <QCoreApplication>

class PluginTest : public QObject
{
    Q_OBJECT
public:
    PluginTest();

private Q_SLOTS:
    void listContainmentActions();
    void listContainmentsOfType();
};

#endif
