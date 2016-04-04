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

QTEST_MAIN(ThemeTest)

