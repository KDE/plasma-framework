/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef THEMETEST_H
#define THEMETEST_H

#include <QTest>

#include "plasma/theme.h"

class ThemeTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();

private Q_SLOTS:
    void testThemeConfig_data();
    void testThemeConfig();
    void testColors();
    void testCompositingChange();

private:
    Plasma::Theme *m_theme;
};

#endif
