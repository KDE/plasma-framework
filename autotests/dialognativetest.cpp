/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dialognativetest.h"

#include "utils.h"

#include <KWindowSystem>

void DialogNativeTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    Plasma::TestUtils::installPlasmaTheme();

    m_cacheDir = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    m_cacheDir.removeRecursively();

    m_dialog = new PlasmaQuick::Dialog;
    m_dialog->setLocation(Plasma::Types::TopEdge);

    m_panel = new QQuickView;
    m_panel->setGeometry(0, 0, 50, 50);
    m_panel->setFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

    m_panel2 = new QQuickView;
    m_panel2->setGeometry(100, 0, 50, 50);
    m_panel2->setFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

    m_panel3 = new QQuickView;
    m_panel3->setGeometry(200, 0, 50, 50);
    m_panel3->setFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

    m_content = new QQuickItem;
    m_content->setWidth(100);
    m_content->setHeight(100);
    m_dialog->setMainItem(m_content);

    m_content2 = new QQuickItem(m_panel3->contentItem());
    m_content2->setWidth(50);
    m_content2->setHeight(25);

    m_panel->show();
    m_panel2->show();
    m_panel3->show();
    KWindowSystem::setType(m_panel->winId(), NET::Dock);
    KWindowSystem::setType(m_panel3->winId(), NET::Dock);
    m_dialog->setVisualParent(m_panel->contentItem());
    m_dialog->show();
}

void DialogNativeTest::cleanupTestCase()
{
    delete m_dialog;
    delete m_panel;
    delete m_panel2;
    delete m_panel3;
    delete m_content;

    m_cacheDir.removeRecursively();
}

void DialogNativeTest::size()
{
    QVERIFY(QTest::qWaitForWindowExposed(m_dialog));

    QCOMPARE(m_content->width(), (qreal)100);
    QCOMPARE(m_content->height(), (qreal)100);
    QCOMPARE(m_dialog->width(), 112);
    QCOMPARE(m_dialog->height(), 112);

    QCOMPARE(m_content2->width(), (qreal)50);
    QCOMPARE(m_content2->height(), (qreal)25);

    QCOMPARE(m_dialog->margins()->property("left").value<qreal>(), (qreal)6.0);
    QCOMPARE(m_dialog->margins()->property("top").value<qreal>(), (qreal)6.0);
    QCOMPARE(m_dialog->margins()->property("right").value<qreal>(), (qreal)6.0);
    QCOMPARE(m_dialog->margins()->property("bottom").value<qreal>(), (qreal)6.0);
}

void DialogNativeTest::position()
{
    QVERIFY(QTest::qWaitForWindowExposed(m_dialog));

    // Find where the outermost test-panel lives. Normally that would be
    // at x,y = (0,0) but if the test is run on a desktop with a
    // left-hand-edge panel, then the test-panel is placed next to it.
    const auto upper_left_x = m_panel->x();
    const auto upper_left_y = m_panel->y();

    QCOMPARE(m_dialog->x(), upper_left_x + 0);
    QCOMPARE(m_dialog->y(), upper_left_y + 49);

    m_dialog->setVisualParent(m_panel2->contentItem());
    QCOMPARE(m_dialog->x(), 69);
    QCOMPARE(m_dialog->y(), 49);

    m_panel3->setMask(QRect(0, 0, 50, 25));
    m_dialog->setVisualParent(m_content2);
    QCOMPARE(m_dialog->x(), 169);
    QCOMPARE(m_dialog->y(), 24);
}

QTEST_MAIN(DialogNativeTest)
