/*
    SPDX-FileCopyrightText: 2014 Marco Martin <mart@kde.org>
    SPDX-FileCopyrightText: 2023 Harald Sitter <sitter@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "dialognativetest.h"

#include "utils.h"

#include <KWindowSystem>

namespace
{
constexpr auto panelHeight = 50;
constexpr auto panelWidth = panelHeight;
constexpr qreal content1Width = 100;
constexpr qreal content1Height = content1Width;
constexpr qreal content2Width = 50;
constexpr qreal content2Height = 25;
} // namespace

void DialogNativeTest::initTestCase()
{
    QStandardPaths::setTestModeEnabled(true);
    Plasma::TestUtils::installPlasmaTheme();

    m_cacheDir = QDir(QStandardPaths::writableLocation(QStandardPaths::CacheLocation));
    m_cacheDir.removeRecursively();

    m_dialog = new PlasmaQuick::Dialog;
    m_dialog->setLocation(Plasma::Types::TopEdge);

    m_panel = new QQuickView;
    m_panel->setColor(Qt::red);
    m_panel->setGeometry(0, 0, panelHeight, panelWidth);
    m_panel->setFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

    m_panel2 = new QQuickView;
    m_panel2->setColor(Qt::green);
    m_panel2->setGeometry(panelWidth * 2, 0, panelHeight, panelWidth);
    m_panel2->setFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

    m_panel3 = new QQuickView;
    m_panel3->setColor(Qt::blue);
    m_panel3->setGeometry(panelWidth * 4, 0, panelHeight, panelWidth);
    m_panel3->setFlags(Qt::FramelessWindowHint | Qt::WindowDoesNotAcceptFocus);

    m_content = new QQuickItem;
    m_content->setWidth(content1Width);
    m_content->setHeight(content1Height);
    m_dialog->setMainItem(m_content);

    m_content2 = new QQuickItem(m_panel3->contentItem());
    m_content2->setWidth(content2Width);
    m_content2->setHeight(content2Height);

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

    QCOMPARE(m_content->width(), content1Width);
    QCOMPARE(m_content->height(), content1Height);

    constexpr qreal themeFixedMargin = 4.0;
    QCOMPARE(m_dialog->margins()->property("left").value<qreal>(), themeFixedMargin);
    QCOMPARE(m_dialog->margins()->property("top").value<qreal>(), themeFixedMargin);
    QCOMPARE(m_dialog->margins()->property("right").value<qreal>(), themeFixedMargin);
    QCOMPARE(m_dialog->margins()->property("bottom").value<qreal>(), themeFixedMargin);

    QCOMPARE(m_dialog->width(), content1Width + themeFixedMargin * 2);
    QCOMPARE(m_dialog->height(), content1Height + themeFixedMargin * 2);

    QCOMPARE(m_content2->width(), content2Width);
    QCOMPARE(m_content2->height(), content2Height);
}

void DialogNativeTest::position()
{
    QVERIFY(QTest::qWaitForWindowExposed(m_dialog));

    // Find where the outermost test-panel lives. Normally that would be
    // at x,y = (0,0) but if the test is run on a desktop with a
    // left-hand-edge panel, then the test-panel is placed next to it.
    const auto upper_left_x = m_panel->x();
    const auto upper_left_y = m_panel->y();

    constexpr auto offset = 1;
    constexpr auto anchorY = panelHeight - offset;

    QCOMPARE(m_dialog->x(), upper_left_x + 0);
    QCOMPARE(m_dialog->y(), upper_left_y + anchorY);

    m_dialog->setVisualParent(m_panel2->contentItem());
    // this derives from the center point of the current panel, I am too lazy to calculate this - sitter, 2023
    QCOMPARE(m_dialog->x(), 71);
    QCOMPARE(m_dialog->y(), anchorY);

    m_panel3->setMask(QRect(0, 0, panelWidth, panelHeight / 2));
    m_dialog->setVisualParent(m_content2);
    QCOMPARE(m_dialog->x(), 171);
    QCOMPARE(m_dialog->y(), panelHeight / 2 - offset);
}

QTEST_MAIN(DialogNativeTest)

#include "moc_dialognativetest.cpp"
