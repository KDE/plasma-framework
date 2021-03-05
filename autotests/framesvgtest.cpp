/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "framesvgtest.h"
#include <QStandardPaths>

void FrameSvgTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    m_cacheDir = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    m_cacheDir.removeRecursively();

    m_frameSvg = new Plasma::FrameSvg;
    m_frameSvg->setImagePath(QFINDTESTDATA("data/background.svgz"));
    QVERIFY(m_frameSvg->isValid());
}

void FrameSvgTest::cleanupTestCase()
{
    delete m_frameSvg;

    m_cacheDir.removeRecursively();
}

void FrameSvgTest::margins()
{
    QCOMPARE(m_frameSvg->marginSize(Plasma::Types::LeftMargin), (qreal)26);
    QCOMPARE(m_frameSvg->marginSize(Plasma::Types::TopMargin), (qreal)26);
    QCOMPARE(m_frameSvg->marginSize(Plasma::Types::RightMargin), (qreal)26);
    QCOMPARE(m_frameSvg->marginSize(Plasma::Types::BottomMargin), (qreal)26);
}

void FrameSvgTest::contentsRect()
{
    m_frameSvg->resizeFrame(QSize(100, 100));
    QCOMPARE(m_frameSvg->contentsRect(), QRectF(26, 26, 48, 48));
}

void FrameSvgTest::repaintBlocked()
{
    // check the properties to be correct even if set during a repaint blocked transaction
    m_frameSvg->setRepaintBlocked(true);
    QVERIFY(m_frameSvg->isRepaintBlocked());

    m_frameSvg->setElementPrefix("prefix");
    m_frameSvg->setEnabledBorders(Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::LeftBorder);
    m_frameSvg->resizeFrame(QSizeF(100, 100));

    m_frameSvg->setRepaintBlocked(false);

    QCOMPARE(m_frameSvg->prefix(), QString("prefix"));
    QCOMPARE(m_frameSvg->enabledBorders(), Plasma::FrameSvg::TopBorder | Plasma::FrameSvg::LeftBorder);
    QCOMPARE(m_frameSvg->frameSize(), QSizeF(100, 100));
}

void FrameSvgTest::setTheme()
{
    // Should not crash

    Plasma::FrameSvg *frameSvg = new Plasma::FrameSvg;
    frameSvg->setImagePath("widgets/background");
    frameSvg->setTheme(new Plasma::Theme("breeze-light", this));
    frameSvg->framePixmap();
    frameSvg->setTheme(new Plasma::Theme("breeze-dark", this));
    frameSvg->framePixmap();
    delete frameSvg;

    frameSvg = new Plasma::FrameSvg;
    frameSvg->setImagePath("widgets/background");
    frameSvg->setTheme(new Plasma::Theme("breeze-light", this));
    frameSvg->framePixmap();
    frameSvg->setTheme(new Plasma::Theme("breeze-dark", this));
    frameSvg->framePixmap();
    delete frameSvg;
}

QTEST_MAIN(FrameSvgTest)
