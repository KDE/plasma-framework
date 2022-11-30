/*
    SPDX-FileCopyrightText: 2016 Marco Martin <mart@kde.org>

    SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "themetest.h"
#include "../src/plasma/private/svg_p.h"
#include <QApplication>
#include <QSignalSpy>
#include <QStandardPaths>

#include <KConfigGroup>
#include <KIconLoader>
#include <KIconTheme>
#include <KWindowSystem>
#include <KX11Extras>

#include <config-plasma.h>
#if HAVE_X11
#include <KSelectionOwner>
#endif
#include <array>

QString cacheIdHash(const Plasma::SvgPrivate::CacheId &id)
{
    static const uint seed = 0x9e3779b9;
    std::array<size_t, 10> parts = {
        ::qHash(id.width),
        ::qHash(id.height),
        ::qHash(id.elementName),
        ::qHash(id.filePath),
        ::qHash(id.status),
        ::qHash(id.devicePixelRatio),
        ::qHash(id.scaleFactor),
        ::qHash(id.colorGroup),
        ::qHash(id.extraFlags),
        ::qHash(id.lastModified),
    };
    return QString::number(qHashRange(parts.begin(), parts.end(), seed));
}

void ThemeTest::initTestCase()
{
    // make our theme in search path
    qputenv("XDG_DATA_DIRS", qgetenv("XDG_DATA_DIRS") + ":" + QFINDTESTDATA("data").toLocal8Bit());

    // set default icon theme to test-theme
    QStandardPaths::setTestModeEnabled(true);

    m_theme = new Plasma::Theme("testtheme", this);
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);

    if (!QDir(configPath).mkpath(QStringLiteral("."))) {
        qFatal("Failed to create test configuration directory.");
    }

    QFile::remove(configPath);

    QIcon::setThemeSearchPaths(QStringList() << QFINDTESTDATA("data/icons"));

    KConfigGroup plasmaConfig(KSharedConfig::openConfig("plasmarc"), "Theme");
    plasmaConfig.writeEntry("name", "default");
    m_svg = new Plasma::Svg();

    KIconTheme::forceThemeForTests("test-theme");
    KSharedConfig::openConfig()->reparseConfiguration();
    KIconTheme::reconfigure();
    KIconLoader::global()->reconfigure(QString());
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

    QVERIFY(iconTheme);
    QVERIFY(iconTheme->isValid());
    QVERIFY2(QFile::exists(iconPath), qPrintable(iconPath));
    m_svg->setImagePath(iconPath);
    QVERIFY(m_svg->isValid());

    m_svg->pixmap(); // trigger the SVG being loaded

    QFileInfo info(iconPath);

    QString cacheId = cacheIdHash(Plasma::SvgPrivate::CacheId{48,
                                                              48,
                                                              iconPath,
                                                              QString(),
                                                              m_svg->status(),
                                                              m_svg->devicePixelRatio(),
                                                              m_svg->scaleFactor(),
                                                              m_svg->colorGroup(),
                                                              0,
                                                              static_cast<uint>(info.lastModified().toSecsSinceEpoch())});

    QPixmap result;
    QVERIFY(m_svg->theme()->findInCache(cacheId, result, info.lastModified().toSecsSinceEpoch()));

    QSignalSpy spy(m_svg, SIGNAL(repaintNeeded()));
    QVERIFY(spy.isValid());

    KIconTheme::forceThemeForTests("test-theme-two");
    // KIconloader needs changesto be emitted manually, ouch.
    for (int i = 0; i < KIconLoader::LastGroup; i++) {
        KIconLoader::emitChange(KIconLoader::Group(i));
    }

    spy.wait();
    // Svg emitting repaintNeeded when something changes in KIconLoader means the svg loaded an icon
    QVERIFY(spy.count() == 1);

    QVERIFY(!m_svg->theme()->findInCache(cacheId, result));
}

void ThemeTest::testThemeConfig_data()
{
    QTest::addColumn<QString>("themeName");

    QTest::addRow("new metadata style theme") << QStringLiteral("testtheme");
#if PLASMA_BUILD_DEPRECATED_SINCE(5, 86)
    QTest::addRow("old metadata format theme") << QStringLiteral("test_old_metadata_format_theme");
#endif
}

void ThemeTest::testThemeConfig()
{
    QFETCH(QString, themeName);
    auto theme = std::make_unique<Plasma::Theme>(themeName, this);
    QCOMPARE(theme->backgroundContrastEnabled(), true);
    QCOMPARE(theme->backgroundContrast(), 0.23);
}

void ThemeTest::testColors()
{
    QCOMPARE(m_theme->color(Plasma::Theme::TextColor, Plasma::Theme::NormalColorGroup), QColor(49, 54, 59));
    QCOMPARE(m_theme->color(Plasma::Theme::BackgroundColor, Plasma::Theme::NormalColorGroup), QColor(239, 240, 241));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightColor, Plasma::Theme::NormalColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::HoverColor, Plasma::Theme::NormalColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::FocusColor, Plasma::Theme::NormalColorGroup), QColor(30, 146, 255));
    QCOMPARE(m_theme->color(Plasma::Theme::LinkColor, Plasma::Theme::NormalColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::VisitedLinkColor, Plasma::Theme::NormalColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightedTextColor, Plasma::Theme::NormalColorGroup), QColor(252, 252, 252));
    QCOMPARE(m_theme->color(Plasma::Theme::PositiveTextColor, Plasma::Theme::NormalColorGroup), QColor(17, 209, 22));
    QCOMPARE(m_theme->color(Plasma::Theme::NeutralTextColor, Plasma::Theme::NormalColorGroup), QColor(201, 206, 59));
    QCOMPARE(m_theme->color(Plasma::Theme::NegativeTextColor, Plasma::Theme::NormalColorGroup), QColor(237, 21, 21));

    QCOMPARE(m_theme->color(Plasma::Theme::TextColor, Plasma::Theme::ButtonColorGroup), QColor(49, 54, 59));
    QCOMPARE(m_theme->color(Plasma::Theme::BackgroundColor, Plasma::Theme::ButtonColorGroup), QColor(239, 240, 241));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightColor, Plasma::Theme::ButtonColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::HoverColor, Plasma::Theme::ButtonColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::FocusColor, Plasma::Theme::ButtonColorGroup), QColor(30, 146, 255));
    QCOMPARE(m_theme->color(Plasma::Theme::LinkColor, Plasma::Theme::ButtonColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::VisitedLinkColor, Plasma::Theme::ButtonColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightedTextColor, Plasma::Theme::ButtonColorGroup), QColor(252, 252, 252));
    QCOMPARE(m_theme->color(Plasma::Theme::PositiveTextColor, Plasma::Theme::ButtonColorGroup), QColor(17, 209, 23));
    QCOMPARE(m_theme->color(Plasma::Theme::NeutralTextColor, Plasma::Theme::ButtonColorGroup), QColor(201, 206, 60));
    QCOMPARE(m_theme->color(Plasma::Theme::NegativeTextColor, Plasma::Theme::ButtonColorGroup), QColor(237, 21, 22));

    QCOMPARE(m_theme->color(Plasma::Theme::TextColor, Plasma::Theme::ViewColorGroup), QColor(49, 54, 59));
    QCOMPARE(m_theme->color(Plasma::Theme::BackgroundColor, Plasma::Theme::ViewColorGroup), QColor(252, 252, 252));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightColor, Plasma::Theme::ViewColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::HoverColor, Plasma::Theme::ViewColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::FocusColor, Plasma::Theme::ViewColorGroup), QColor(30, 146, 255));
    QCOMPARE(m_theme->color(Plasma::Theme::LinkColor, Plasma::Theme::ViewColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::VisitedLinkColor, Plasma::Theme::ViewColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightedTextColor, Plasma::Theme::ViewColorGroup), QColor(252, 252, 252));
    QCOMPARE(m_theme->color(Plasma::Theme::PositiveTextColor, Plasma::Theme::ViewColorGroup), QColor(17, 209, 24));
    QCOMPARE(m_theme->color(Plasma::Theme::NeutralTextColor, Plasma::Theme::ViewColorGroup), QColor(201, 206, 61));
    QCOMPARE(m_theme->color(Plasma::Theme::NegativeTextColor, Plasma::Theme::ViewColorGroup), QColor(237, 21, 23));

    QCOMPARE(m_theme->color(Plasma::Theme::TextColor, Plasma::Theme::ComplementaryColorGroup), QColor(239, 240, 241));
    QCOMPARE(m_theme->color(Plasma::Theme::BackgroundColor, Plasma::Theme::ComplementaryColorGroup), QColor(49, 54, 59));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightColor, Plasma::Theme::ComplementaryColorGroup), QColor(61, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::HoverColor, Plasma::Theme::ComplementaryColorGroup), QColor(71, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::FocusColor, Plasma::Theme::ComplementaryColorGroup), QColor(40, 146, 255));
    QCOMPARE(m_theme->color(Plasma::Theme::LinkColor, Plasma::Theme::ComplementaryColorGroup), QColor(71, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::VisitedLinkColor, Plasma::Theme::ComplementaryColorGroup), QColor(71, 174, 230));
    QCOMPARE(m_theme->color(Plasma::Theme::HighlightedTextColor, Plasma::Theme::ComplementaryColorGroup), QColor(252, 252, 252));
    QCOMPARE(m_theme->color(Plasma::Theme::PositiveTextColor, Plasma::Theme::ComplementaryColorGroup), QColor(17, 209, 25));
    QCOMPARE(m_theme->color(Plasma::Theme::NeutralTextColor, Plasma::Theme::ComplementaryColorGroup), QColor(201, 206, 62));
    QCOMPARE(m_theme->color(Plasma::Theme::NegativeTextColor, Plasma::Theme::ComplementaryColorGroup), QColor(237, 21, 24));
}

void ThemeTest::testCompositingChange()
{
    // this test simulates the compositing change on X11
#if HAVE_X11
    if (!KWindowSystem::isPlatformX11()) {
        QSKIP("Test is only for X11");
    }
    QVERIFY(!KX11Extras::compositingActive());

    // image path should give us an opaque variant
    QVERIFY(m_theme->imagePath(QStringLiteral("element")).endsWith(QLatin1String("/desktoptheme/testtheme/opaque/element.svg")));

    QSignalSpy themeChangedSpy(m_theme, &Plasma::Theme::themeChanged);
    QVERIFY(themeChangedSpy.isValid());

    // fake the compositor
    QSignalSpy compositingChangedSpy(KX11Extras::self(), &KX11Extras::compositingChanged);
    QVERIFY(compositingChangedSpy.isValid());
    std::unique_ptr<KSelectionOwner> compositorSelection(new KSelectionOwner("_NET_WM_CM_S0"));
    QSignalSpy claimedSpy(compositorSelection.get(), &KSelectionOwner::claimedOwnership);
    QVERIFY(claimedSpy.isValid());
    compositorSelection->claim(true);
    QVERIFY(claimedSpy.wait());

    QCOMPARE(compositingChangedSpy.count(), 1);
    QVERIFY(KX11Extras::compositingActive());
    QVERIFY(themeChangedSpy.wait());
    QCOMPARE(themeChangedSpy.count(), 1);
    QVERIFY(m_theme->imagePath(QStringLiteral("element")).endsWith(QLatin1String("/desktoptheme/testtheme/element.svg")));

    // remove compositor again
    compositorSelection.reset();
    QVERIFY(compositingChangedSpy.wait());
    QCOMPARE(compositingChangedSpy.count(), 2);
    QVERIFY(!KX11Extras::compositingActive());
    QVERIFY(themeChangedSpy.wait());
    QCOMPARE(themeChangedSpy.count(), 2);
    QVERIFY(m_theme->imagePath(QStringLiteral("element")).endsWith(QLatin1String("/desktoptheme/testtheme/opaque/element.svg")));
#endif
}

QTEST_MAIN(ThemeTest)
