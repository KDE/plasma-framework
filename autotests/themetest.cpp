/********************************************************************************
*   Copyright 2016 Marco Martin <mart@kde.org>                                  *
*                                                                               *
*   This library is free software; you can redistribute it and/or               *
*   modify it under the terms of the GNU Library General Public                 *
*   License as published by the Free Software Foundation; either                *
*   version 2 of the License, or (at your option) any later version.            *
*                                                                               *
*   This library is distributed in the hope that it will be useful,             *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of              *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU            *
*   Library General Public License for more details.                            *
*                                                                               *
*   You should have received a copy of the GNU Library General Public License   *
*   along with this library; see the file COPYING.LIB.  If not, write to        *
*   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,        *
*   Boston, MA 02110-1301, USA.                                                 *
*********************************************************************************/

#include "themetest.h"
#include <QStandardPaths>
#include <QApplication>

#include <KIconLoader>
#include <KIconTheme>

#define QLSEP QLatin1Char('_')
#define CACHE_ID_WITH_SIZE(size, id, devicePixelRatio) QString::number(int(size.width())) % QLSEP % QString::number(int(size.height())) % QLSEP % id % QLSEP % QLSEP % QString::number(int(devicePixelRatio))

void ThemeTest::initTestCase()
{
    // make our theme in search path
    qputenv("XDG_DATA_DIRS",
            qgetenv("XDG_DATA_DIRS") + ":" + QFINDTESTDATA("data").toLocal8Bit());

    // set default icon theme to test-theme
    QStandardPaths::setTestModeEnabled(true);

    m_theme = new Plasma::Theme("testtheme", this);
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);

    if(!QDir(configPath).mkpath(QStringLiteral("."))) {
        qFatal("Failed to create test configuration directory.");
    }

    QFile::remove(configPath);

    QIcon::setThemeSearchPaths(QStringList()<<QFINDTESTDATA("data/icons"));

    KConfigGroup cg(KSharedConfig::openConfig("kdeglobals"), "Icons");
    cg.writeEntry("Theme", "test-theme");
    KConfigGroup plasmaConfig(KSharedConfig::openConfig("plasmarc"), "Theme");
    plasmaConfig.writeEntry("name", "default");
    m_svg = new Plasma::Svg();
}

void ThemeTest::cleanupTestCase()
{
//    m_testIconsDir.removeRecursively();
    delete m_svg;
}

void ThemeTest::loadSvgIcon()
{
    const auto *iconTheme = KIconLoader::global()->theme();
    QString iconPath;
    if (iconTheme) {
        iconPath = iconTheme->iconPath(QLatin1String("konversation.svg"), 48, KIconLoader::MatchBest);
    }

    m_svg->setImagePath(iconPath);
    QVERIFY(m_svg->isValid());

    m_svg->pixmap(); //trigger the SVG being loaded

    QString cacheId = CACHE_ID_WITH_SIZE(QSize(48, 48), iconPath, m_svg->devicePixelRatio()) % QLSEP % QString::number(m_svg->colorGroup());

    QPixmap result;
    QVERIFY(m_svg->theme()->findInCache(cacheId, result));

    QSignalSpy spy(m_svg, SIGNAL(repaintNeeded()));
    QVERIFY(spy.isValid());

    KConfigGroup cg(KSharedConfig::openConfig("kdeglobals"), "Icons");
    cg.writeEntry("Theme", "test-theme-two");
    cg.sync();
    // KIconloader needs changesto be emitted manually, ouch.
    for (int i=0; i < KIconLoader::LastGroup; i++) {
        KIconLoader::emitChange(KIconLoader::Group(i));
    }

    spy.wait();
    //Svg emitting repaintNeeded when something changes in KIconLoader means the svg loaded an icon
    QVERIFY(spy.count() == 1);

    QVERIFY(!m_svg->theme()->findInCache(cacheId, result));
}

void ThemeTest::testColors()
{
    QCOMPARE(m_theme->color(Plasma::Theme::TextColor,
                            Plasma::Theme::NormalColorGroup), QColor(49,54,59));
    QCOMPARE(m_theme->color(Plasma::Theme::BackgroundColor,
                            Plasma::Theme::NormalColorGroup), QColor(239,240,241));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightColor,
                            Plasma::Theme::NormalColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::HoverColor,
                            Plasma::Theme::NormalColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::FocusColor,
                            Plasma::Theme::NormalColorGroup), QColor(30,146,255));
    QCOMPARE(m_theme->color(Plasma::Theme::LinkColor,
                            Plasma::Theme::NormalColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::VisitedLinkColor,
                            Plasma::Theme::NormalColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightedTextColor,
                            Plasma::Theme::NormalColorGroup).name(), QColor(163,214,251).name());
    QCOMPARE(m_theme->color(Plasma::Theme::PositiveTextColor,
                            Plasma::Theme::NormalColorGroup), QColor(17,209,22));
    QCOMPARE(m_theme->color(Plasma::Theme::NeutralTextColor,
                            Plasma::Theme::NormalColorGroup), QColor(201,206,59));
    QCOMPARE(m_theme->color(Plasma::Theme::NegativeTextColor,
                            Plasma::Theme::NormalColorGroup), QColor(237,21,21));

    QCOMPARE(m_theme->color(Plasma::Theme::TextColor,
                            Plasma::Theme::ButtonColorGroup), QColor(49,54,59));
    QCOMPARE(m_theme->color(Plasma::Theme::BackgroundColor,
                            Plasma::Theme::ButtonColorGroup), QColor(239,240,241));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightColor,
                            Plasma::Theme::ButtonColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::HoverColor,
                            Plasma::Theme::ButtonColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::FocusColor,
                            Plasma::Theme::ButtonColorGroup), QColor(30,146,255));
    QCOMPARE(m_theme->color(Plasma::Theme::LinkColor,
                            Plasma::Theme::ButtonColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::VisitedLinkColor,
                            Plasma::Theme::ButtonColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightedTextColor,
                            Plasma::Theme::ButtonColorGroup).name(), QColor(163,214,251).name());
    QCOMPARE(m_theme->color(Plasma::Theme::PositiveTextColor,
                            Plasma::Theme::ButtonColorGroup), QColor(17,209,23));
    QCOMPARE(m_theme->color(Plasma::Theme::NeutralTextColor,
                            Plasma::Theme::ButtonColorGroup), QColor(201,206,60));
    QCOMPARE(m_theme->color(Plasma::Theme::NegativeTextColor,
                            Plasma::Theme::ButtonColorGroup), QColor(237,21,22));

    QCOMPARE(m_theme->color(Plasma::Theme::TextColor,
                            Plasma::Theme::ViewColorGroup), QColor(49,54,59));
    QCOMPARE(m_theme->color(Plasma::Theme::BackgroundColor,
                            Plasma::Theme::ViewColorGroup), QColor(252,252,252));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightColor,
                            Plasma::Theme::ViewColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::HoverColor,
                            Plasma::Theme::ViewColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::FocusColor,
                            Plasma::Theme::ViewColorGroup), QColor(30,146,255));
    QCOMPARE(m_theme->color(Plasma::Theme::LinkColor,
                            Plasma::Theme::ViewColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::VisitedLinkColor,
                            Plasma::Theme::ViewColorGroup), QColor(61,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightedTextColor,
                            Plasma::Theme::ViewColorGroup).name(), QColor(163,214,251).name());
    QCOMPARE(m_theme->color(Plasma::Theme::PositiveTextColor,
                            Plasma::Theme::ViewColorGroup), QColor(17,209,24));
    QCOMPARE(m_theme->color(Plasma::Theme::NeutralTextColor,
                            Plasma::Theme::ViewColorGroup), QColor(201,206,61));
    QCOMPARE(m_theme->color(Plasma::Theme::NegativeTextColor,
                            Plasma::Theme::ViewColorGroup), QColor(237,21,23));

    QCOMPARE(m_theme->color(Plasma::Theme::TextColor,
                            Plasma::Theme::ComplementaryColorGroup), QColor(239,240,241));
    QCOMPARE(m_theme->color(Plasma::Theme::BackgroundColor,
                            Plasma::Theme::ComplementaryColorGroup), QColor(49,54,59));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightColor,
                            Plasma::Theme::ComplementaryColorGroup), QColor(71,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::HoverColor,
                            Plasma::Theme::ComplementaryColorGroup), QColor(71,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::FocusColor,
                            Plasma::Theme::ComplementaryColorGroup), QColor(40,146,255));
    QCOMPARE(m_theme->color(Plasma::Theme::LinkColor,
                            Plasma::Theme::ComplementaryColorGroup), QColor(71,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::VisitedLinkColor,
                            Plasma::Theme::ComplementaryColorGroup), QColor(71,174,230));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightedTextColor,
                            Plasma::Theme::ComplementaryColorGroup).name(), QColor(169,214,249).name());
    QCOMPARE(m_theme->color(Plasma::Theme::PositiveTextColor,
                            Plasma::Theme::ComplementaryColorGroup), QColor(17,209,25));
    QCOMPARE(m_theme->color(Plasma::Theme::NeutralTextColor,
                            Plasma::Theme::ComplementaryColorGroup), QColor(201,206,62));
    QCOMPARE(m_theme->color(Plasma::Theme::NegativeTextColor,
                            Plasma::Theme::ComplementaryColorGroup), QColor(237,21,24));
}


QTEST_MAIN(ThemeTest)

