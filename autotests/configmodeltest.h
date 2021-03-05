/*
    SPDX-FileCopyrightText: 2016 David Rosca <nowrep@gmail.com>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#pragma once

#include <QTest>

namespace Plasma
{
class Applet;
}

class ConfigModelTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void configSchemeFromPackage();
    void emptySourceWithApplet();
    void notEmptySourceWithApplet();
};
