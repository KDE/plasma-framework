/*
    SPDX-FileCopyrightText: 2013 Sebastian Kügler <sebas@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dpitest.h"

#include <QDebug>

#include <plasma/theme.h>

#include <QCommandLineParser>
#include <QStringList>
#include <QTimer>
#include <iomanip>
#include <iostream>

namespace Plasma
{
class DPITestPrivate
{
public:
    QString pluginName;
    QCommandLineParser *parser;
};

DPITest::DPITest(int &argc, char **argv, QCommandLineParser *parser)
    : QGuiApplication(argc, argv)
{
    d = new DPITestPrivate;
    d->parser = parser;
    QTimer::singleShot(0, this, &DPITest::runMain);
}

DPITest::~DPITest()
{
    delete d;
}

void DPITest::runMain()
{
    qDebug() << "DPI test runs: ";
    exit(0);
    return;
}

}

#include "moc_dpitest.cpp"
