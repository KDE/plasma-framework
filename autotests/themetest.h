/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef THEMETEST_H
#define THEMETEST_H

#include <QTest>

#include "plasma/svg.h"
#include "plasma/theme.h"

class ThemeTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void loadSvgIcon();
    void testColors();
    void testCompositingChange();

private:
    Plasma::Svg *m_svg;
    Plasma::Theme *m_theme;
};

#endif
