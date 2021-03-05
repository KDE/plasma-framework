/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef FRAMESVGTEST_H
#define FRAMESVGTEST_H

#include <QTest>

#include "plasma/framesvg.h"

class FrameSvgTest : public QObject
{
    Q_OBJECT

public Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();

private Q_SLOTS:
    void margins();
    void contentsRect();
    void setTheme();
    void repaintBlocked();

private:
    Plasma::FrameSvg *m_frameSvg;
    QDir m_cacheDir;
};

#endif
