/*
    SPDX-FileCopyrightText: 2012 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef DPITEST_H
#define DPITEST_H

#include <QCoreApplication>
#include <QGuiApplication>

class QCommandLineParser;

namespace Plasma
{
class DPITestPrivate;

class DPITest : public QGuiApplication
{
    Q_OBJECT

public:
    DPITest(int &argc, char **argv, QCommandLineParser *parser);
    ~DPITest() override;

public Q_SLOTS:
    void runMain();

private:
    DPITestPrivate *d;
};

}

#endif
