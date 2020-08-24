/*
    SPDX-FileCopyrightText: 2011 Aaron Seigo <aseigo@kde.org>

    SPDX-License-Identifier: BSD-2-Clause
*/

#ifndef SIMPLEENGINE_H
#define SIMPLEENGINE_H

#include <Plasma/DataEngine>

class SimpleEngine : public Plasma::DataEngine
{
    Q_OBJECT

public:
    SimpleEngine(QObject *parent, const QVariantList &args);

    void init();
};

#endif
